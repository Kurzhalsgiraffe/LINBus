//#define F_CPU 16000000UL // CPU-Frequenz des ATmega328P (16 MHz)
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

const unsigned int buffsize = 50;
uint8_t capture[buffsize];
uint8_t values[buffsize];

volatile uint8_t state = 0x0;
volatile uint16_t ovf = 0;
volatile uint8_t count = 0;

ISR(PCINT2_vect) {
  state = readBitD(RX_PIN);
  TCCR0B = 0;
  count = TCNT0;
  TCNT0 = 0;
  ovf = 0;
  TCCR0B = (1<<CS02) | (1 << CS00); // start timer
}

ISR(TIMER0_OVF_vect) {
  ovf += 1;
}

int slave_init() {
  cli();

  // LED Init
  DDRB |= (1 << BLUE_LED_PIN); //OUT
  DDRB |= (1 << GREEN_LED_PIN); //OUT
  DDRB |= (1 << RED_LED_PIN); //OUT

  // COM Pin Init
  DDRD &= ~(1 << RX_PIN); //IN
  DDRD |= (1 << TX_PIN); //OUT
  DDRD |= (1 << CS_PIN); //OUT

  PORTD |= (1 << CS_PIN); //HIGH

  DDRC |= (1 << HEAD_0_PIN); //OUT
  DDRC |= (1 << HEAD_1_PIN); //OUT

  PORTC &= ~(1 << HEAD_0_PIN); //LOW
  PORTC &= ~(1 << HEAD_1_PIN); //LOW

  // https://www.arxterra.com/11-atmega328p-external-interrupts/
  // https://stackoverflow.com/questions/70049553/best-way-to-handle-multiple-pcint-in-avr
  PCICR |= (1 << PCIE2);   // group 2
  PCMSK2 |= (1 << PCINT16);   // set PCINT16 to interrupt


  // https://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/Die_Timer_und_Z%C3%A4hler_des_AVR
  TCCR0A = 0;
  //TCCR0B = (1<<CS02) | (1 << CS00); // start timer Prescaler 1024
  TCCR0B = (1<<CS02) | (1 << CS00); // start timer Prescaler 1024
  TIMSK0 |= (1<<TOIE0); // overflow erlaubgen
  //TIFR0  = 1<<TOV0;
  // gibt einen 16-bit counter
  // siehe TCCR1A
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
    PORTB |= (1 << pin);  // LED einschalten
  } else {
    PORTB &= ~(1 << pin); // LED ausschalten
  }
}

void pulse(uint8_t width) {
  if(width == 1) {
    writeBitC(HEAD_0_PIN, 1);
    writeBitC(HEAD_1_PIN, 1);
    _delay_us((1000000 * 5) / (2*BAUD));
    writeBitC(HEAD_0_PIN, 0);
    writeBitC(HEAD_1_PIN, 0);
    _delay_us((1000000 * 5) / (2*BAUD));
  } else {
    writeBitC(HEAD_0_PIN, 1);
    writeBitC(HEAD_1_PIN, 1);
    _delay_us((1000000 * 5) / (BAUD));
    writeBitC(HEAD_0_PIN, 0);
    writeBitC(HEAD_1_PIN, 0);
    _delay_us((1000000 * 5) / (BAUD));
  }
}

void sendbyte(uint8_t byte) {
  for(uint8_t i = 0; i < 8; i++) {
    writeBitC(HEAD_0_PIN, 0);
    writeBitC(HEAD_1_PIN, 0);
    _delay_us((1000000 * 5) / (2*BAUD));
  }

  writeBitC(HEAD_0_PIN, 1);
  writeBitC(HEAD_1_PIN, 1);
  _delay_us((1000000 * 5) / (2*BAUD));

  for(uint8_t i = 0; i < 8; i++) {

    if (((byte >> i) & 0x01) == 0x01) {
      writeBitC(HEAD_0_PIN, 1);
      writeBitC(HEAD_1_PIN, 1);
      _delay_us((1000000 * 5) / (2*BAUD));
    } else {
      writeBitC(HEAD_0_PIN, 0);
      writeBitC(HEAD_1_PIN, 0);
      _delay_us((1000000 * 5) / (2*BAUD));
    }
  }

  writeBitC(HEAD_0_PIN, 1);
  writeBitC(HEAD_1_PIN, 1);
  _delay_us((1000000 * 5) / (2*BAUD));
  writeBitC(HEAD_0_PIN, 1);
  writeBitC(HEAD_1_PIN, 1);
  _delay_us((1000000 * 5) / (2*BAUD));
}

uint8_t converttickstonum(uint8_t num) {
    const uint8_t thresholds[] = {
        4, 12, 20, 28, 38, 45, 55, 64, 72, 80, 88, 95, 105, 115
    };

    for (uint8_t i = 0; i < sizeof(thresholds); i++) {
        if (num <= thresholds[i]) {
            return i;
        }
    }
    return 0x0;
}

uint8_t converttickstonum2(uint8_t ticks) {
    return static_cast<uint8_t>(round(ticks / 8.4734));
}

uint8_t getsyncbytefromheader(uint8_t ci) {
  uint32_t temp = 0;
  uint8_t pos = 0;
  for(uint8_t i = 2; i < ci; i++) {
    uint8_t num = converttickstonum(capture[i]);
    for(uint8_t j = 0; j < num; j++) {
      if (values[i] == 1) {
	    temp |= 0x00000001 << pos;
      }
      pos++;
    }
  }
  temp &= 0x03FC00;
  return (uint8_t)(temp >> 10);
}

uint8_t getpidbytefromheader(uint8_t ci) {
  uint32_t temp = 0;
  uint8_t pos = 0;
  for(uint8_t i = 2; i < ci; i++) {
    uint8_t num = converttickstonum(capture[i]);
    for(uint8_t j = 0; j < num; j++) {
      if (values[i] == 1) {
	    temp |= 0x00000001 << pos;
      }
      pos++;
    }
  }
  temp &= 0x01FE;
  return (uint8_t)(temp >> 1);
}

int main() {
  slave_init();

  uint8_t currentstate = 0;

  //float factor = 0;
  uint8_t ci = 0;


  for (uint8_t i = 0; i < buffsize; i++) {
    capture[i] = 0x0;
    values[i] = 0x0;
  }

  while(1) {

    if(ci >= buffsize) {
      while(1) {

	led(RED_LED_PIN, 1);
	_delay_us(100000);
	led(RED_LED_PIN, 0);
	_delay_us(100000);
      }

    }

    if(currentstate == 0 && state == 0x1) {

      currentstate = 1;

      if (count > 100) {
	ci = 0;
	//factor = 13 / count;
      }

      capture[ci] = count;
      values[ci] = 1;
      ci++;

    }
    if (currentstate == 1 && state == 0x0) {

      currentstate = 0;

      //pulse(2);

      if (count > 100) {
	ci = 0;
	//factor = 13 / count;
      }
      capture[ci] = count;
      values[ci] = 0;
      ci++;
    }
    if (currentstate == 1 && state == 0x1) {
      if (ovf == 1) {

	/*sendbyte(capture[12]);
	sendbyte(capture[13]);
	sendbyte(capture[14]);
	sendbyte(capture[15]);*/

	//sendbyte(ci);
	sendbyte(getsyncbytefromheader(ci));
	//sendbyte(getpidbytefromheader(ci));
	//sendbyte(numtics);
      }

      //for (uint8_t i = 0; i < buffsize; i++) {
      //capture[i] = 0x0;
      //}
    }
  }


#ifdef BLUBBER
  uint16_t result = 0xFFFF;
  while (1) {
    while(1) {
      uint8_t bit = readBitD(RX_PIN);
      if (bit == 0) {
	break;
      }
    }
    while(1) {
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
      _delay_us((1000000 * 10.045) / (2*BAUD));
      writeBitC(HEAD_0_PIN, 0);
      writeBitC(HEAD_1_PIN, 0);
      _delay_us((1000000 * 10.045) / (2*BAUD));

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
      _delay_us((1000000 * 10.045) / (2*BAUD));
      writeBitC(HEAD_0_PIN, 0);
      writeBitC(HEAD_1_PIN, 0);
      _delay_us((1000000 * 10.045) / (2*BAUD));

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
