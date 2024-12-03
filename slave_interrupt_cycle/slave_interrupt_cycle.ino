// #define F_CPU 16000000UL // CPU-Frequenz des ATmega328P (16 MHz)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <util/delay.h>

// LED Pins
#define BLUE_LED_PIN PB0
#define GREEN_LED_PIN PB1
#define RED_LED_PIN PB2

// Head Pins
#define HEAD_0_PIN PC0
#define HEAD_1_PIN PC1

// Read Transmit Pins
#define RX_PIN PD0
#define TX_PIN PD1
#define CS_PIN PD4

const uint16_t BAUD = 19200;

const unsigned int BUFFSIZE = 64;
const unsigned int LIN_SEGMENT_LENGTH = 10;
const unsigned int LIN_MESSAGE_LENGTH = 5 * LIN_SEGMENT_LENGTH;
uint8_t buffer[BUFFSIZE];
uint8_t linSync[LIN_SEGMENT_LENGTH];
uint8_t linPid[LIN_SEGMENT_LENGTH];
uint8_t linLsb[LIN_SEGMENT_LENGTH];
uint8_t linMsb[LIN_SEGMENT_LENGTH];
uint8_t linChecksum[LIN_SEGMENT_LENGTH];

volatile uint8_t state = 0x0;
volatile uint16_t timer_overflow_counter = 0;
volatile uint8_t timer_value = 0;

uint8_t zeroCount;
int8_t lastLinBreakIndex;

uint8_t readBitD(uint8_t);

ISR(PCINT2_vect) {
  state = readBitD(RX_PIN);
  TCCR0B = 0; // Stop timer
  timer_value = TCNT0; // Get current timer value
  TCNT0 = 0; // Reset timer value
  timer_overflow_counter = 0; // Reset overflow counter
  TCCR0B = (1 << CS02) | (1 << CS00); // Set the timer prescaler to 1024; Timer ticks at 16,000,000 / 1024 = 15,625 Hz
}

ISR(TIMER0_OVF_vect) {
  timer_overflow_counter += 1; // When the 8-Bit Timer (TCNT0) overflows, this ISR is triggered
}

int slave_init() {
  cli();

  // LED Init
  DDRB |= (1 << BLUE_LED_PIN);  // OUT
  DDRB |= (1 << GREEN_LED_PIN); // OUT
  DDRB |= (1 << RED_LED_PIN);   // OUT

  // COM Pin Init
  DDRD &= ~(1 << RX_PIN); // IN
  DDRD |= (1 << TX_PIN);  // OUT
  DDRD |= (1 << CS_PIN);  // OUT

  PORTD |= (1 << CS_PIN); // HIGH

  DDRC |= (1 << HEAD_0_PIN); // OUT
  DDRC |= (1 << HEAD_1_PIN); // OUT

  PORTC &= ~(1 << HEAD_0_PIN); // LOW
  PORTC &= ~(1 << HEAD_1_PIN); // LOW

  // https://www.arxterra.com/11-atmega328p-external-interrupts/
  // https://stackoverflow.com/questions/70049553/best-way-to-handle-multiple-pcint-in-avr
  PCICR |= (1 << PCIE2);    // group 2
  PCMSK2 |= (1 << PCINT16); // set PCINT16 to interrupt

  // Timer Registers
  // https://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/Die_Timer_und_Z%C3%A4hler_des_AVR
  TCCR0A = 0;
  TCCR0B = (1 << CS02) | (1 << CS00); // Set the timer prescaler to 1024; Timer ticks at 16,000,000 / 1024 = 15,625 Hz
  TIMSK0 |= (1 << TOIE0);             // overflow erlauben

  sei();
  return 0;
}

void writeBitD(uint8_t pin, uint8_t value) {
  if (value) {
    PORTD |= (1 << pin); // Write 1
  } else {
    PORTD &= ~(1 << pin); // Write 0
  }
}

void writeBitC(uint8_t pin, uint8_t value) {
  if (value) {
    PORTC |= (1 << pin); // Write 1
  } else {
    PORTC &= ~(1 << pin); // Write 0
  }
}

uint8_t readBitD(uint8_t pin) {
  return (PIND & (1 << pin)) ? 0x1 : 0x0;
}

uint8_t calculate_checksum(uint8_t data_low, uint8_t data_high) {
  uint16_t checksum = 0;
  checksum += data_low;
  checksum += data_high;
  return checksum & 0xFF;
}

void led(uint8_t pin, uint8_t value) {
  if (value) {
    PORTB |= (1 << pin); // LED einschalten
  } else {
    PORTB &= ~(1 << pin); // LED ausschalten
  }
}

void pulse(uint8_t width) {
  if (width == 1) {
    writeBitC(HEAD_0_PIN, 1);
    writeBitC(HEAD_1_PIN, 1);
    _delay_us((1000000 * 5) / (2 * BAUD));
    writeBitC(HEAD_0_PIN, 0);
    writeBitC(HEAD_1_PIN, 0);
    _delay_us((1000000 * 5) / (2 * BAUD));
  }
  else {
    writeBitC(HEAD_0_PIN, 1);
    writeBitC(HEAD_1_PIN, 1);
    _delay_us((1000000 * 5) / (BAUD));
    writeBitC(HEAD_0_PIN, 0);
    writeBitC(HEAD_1_PIN, 0);
    _delay_us((1000000 * 5) / (BAUD));
  }
}

void sendbyte(uint8_t byte) {
  for (uint8_t i = 0; i < 8; i++) {
    writeBitC(HEAD_0_PIN, 0);
    writeBitC(HEAD_1_PIN, 0);
    _delay_us((1000000 * 5) / (2 * BAUD));
  }

  writeBitC(HEAD_0_PIN, 1);
  writeBitC(HEAD_1_PIN, 1);
  _delay_us((1000000 * 5) / (2 * BAUD));

  for (uint8_t i = 0; i < 8; i++) {

    if (((byte >> i) & 0x01) == 0x01) {
      writeBitC(HEAD_0_PIN, 1);
      writeBitC(HEAD_1_PIN, 1);
      _delay_us((1000000 * 5) / (2 * BAUD));
    }
    else {
      writeBitC(HEAD_0_PIN, 0);
      writeBitC(HEAD_1_PIN, 0);
      _delay_us((1000000 * 5) / (2 * BAUD));
    }
  }

  writeBitC(HEAD_0_PIN, 1);
  writeBitC(HEAD_1_PIN, 1);
  _delay_us((1000000 * 5) / (2 * BAUD));
  writeBitC(HEAD_0_PIN, 1);
  writeBitC(HEAD_1_PIN, 1);
  _delay_us((1000000 * 5) / (2 * BAUD));
}

uint8_t convertTicksToBitNum(uint8_t ticks) {
  uint8_t num = (ticks * 10000) / 8474;
  if (num % 10 > 5) {
    return (num / 10) + 1;
  }
  return (num / 10);
}

int main() {
  slave_init();

  uint8_t currentstate = 0;
  uint8_t write_pointer = 0;

  for (uint8_t i = 0; i < BUFFSIZE; i++) { // Initialising arrays with Zeros
    buffer[i] = 0x1;
  }

  while (1) {
    zeroCount = 0;
    lastLinBreakIndex = -1;

    if (currentstate == 0 && state == 0x1) { // Rising Edge (Low -> High)
      currentstate = 1;

      uint8_t num = convertTicksToBitNum(timer_value);

      for (uint8_t i=0; i<num; i++) {
        buffer[write_pointer] = 0;
        write_pointer++;

        if (write_pointer == BUFFSIZE) {
          write_pointer = 0;
        }
      }
    }

    if (currentstate == 1 && state == 0x0) { // Falling Edge (High -> Low)
      currentstate = 0;

      uint8_t num = convertTicksToBitNum(timer_value);

      for (uint8_t i=0; i<num; i++) {
        buffer[write_pointer] = 1;
        write_pointer++;

        if (write_pointer == BUFFSIZE) {
          write_pointer = 0;
        }
      }
    }

    for (uint8_t i = 0; i < BUFFSIZE * 2; i++) {
        uint8_t idx = i % BUFFSIZE;

        if (buffer[idx] == 0) {
            zeroCount++;
        } else {
            zeroCount = 0;
        }

        if (zeroCount >= 13 && buffer[(idx + 1) % BUFFSIZE] == 1) {
            lastLinBreakIndex = (idx + 1) % BUFFSIZE;
            break;
        }
    }

    if (lastLinBreakIndex != -1) {
        // Ensure that the message is complete in the buffer
        if ((write_pointer - 1 + BUFFSIZE - lastLinBreakIndex) & (BUFFSIZE - 1) >= LIN_MESSAGE_LENGTH) { // (write_pointer -1 + BUFFSIZE - lastLinBreakIndex) % BUFFSIZE, switched from Modulo to bit operation (only works if it is modulo power of 2)
  	        uint16_t temp1 = 0;
  	        uint16_t temp0 = 0;
            // Read X, Y, Z from the buffer
            for (uint8_t j = 0; j < LIN_SEGMENT_LENGTH; ++j) {
                linSync[j] = buffer[(lastLinBreakIndex + 1 + j) % BUFFSIZE];
                linPid[j] = buffer[(lastLinBreakIndex + 1 + LIN_SEGMENT_LENGTH + j) % BUFFSIZE];
                linLsb[j] = buffer[(lastLinBreakIndex + 1 + 2 * LIN_SEGMENT_LENGTH + j) % BUFFSIZE];
                linMsb[j] = buffer[(lastLinBreakIndex + 1 + 3 * LIN_SEGMENT_LENGTH + j) % BUFFSIZE];
                linChecksum[j] = buffer[(lastLinBreakIndex + 1 + 4 * LIN_SEGMENT_LENGTH + j) % BUFFSIZE];
                if (linSync[j] == 1) {
                  temp0 |= (0x0001 << j);
                }
                if (linPid[j] == 1) {
                  temp1 |= (0x0001 << j);
                }
            }
            sendbyte((0x01FE & temp0) >> 1);
            sendbyte((0x01FE & temp1) >> 1);
        }
    }
  }

#ifdef BLUBBER
  uint16_t result = 0xFFFF;
  while (1) {
    while (1) {
      uint8_t bit = readBitD(RX_PIN);
      if (bit == 0) {
        break;
      }
    }
    while (1) {
      uint8_t bit = readBitD(RX_PIN);
      if (bit == 1) {
        break;
      }
    }
    while (1) {
      uint8_t bit = readBitD(RX_PIN);
      result <<= 1;
      result |= (bit & 0x01);

      writeBitC(HEAD_0_PIN, 1);
      writeBitC(HEAD_1_PIN, 1);
      _delay_us((1000000 * 10.045) / (2 * BAUD));
      writeBitC(HEAD_0_PIN, 0);
      writeBitC(HEAD_1_PIN, 0);
      _delay_us((1000000 * 10.045) / (2 * BAUD));

      if ((result & 0x3FFF) == 0x0001) {

        break;
      }
    }
    result = 0xFFFF;
    while (1) {
      uint8_t bit = readBitD(RX_PIN);
      result <<= 1;
      result |= (bit & 0x01);

      writeBitC(HEAD_0_PIN, 1);
      writeBitC(HEAD_1_PIN, 1);
      _delay_us((1000000 * 10.045) / (2 * BAUD));
      writeBitC(HEAD_0_PIN, 0);
      writeBitC(HEAD_1_PIN, 0);
      _delay_us((1000000 * 10.045) / (2 * BAUD));

      if ((result & 0x007F) == 0x0055) {
        /*led(RED_LED_PIN, 1);
          _delay_us(100000);
          led(RED_LED_PIN, 0);
          _delay_us(100000);*/
        break;
      }
    }
  }
#endif
  return 0;
}
