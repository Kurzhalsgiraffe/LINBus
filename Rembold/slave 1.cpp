//#define F_CPU 16000000UL // CPU-Frequenz des ATmega328P (16 MHz)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <util/delay.h>
#include "slave.hpp"


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

const unsigned int bufsize = 130;
  
uint8_t capture[bufsize];

volatile uint8_t state = 0x0;
volatile uint16_t ovf = 0;
volatile uint16_t ovftemp = 0;
volatile uint8_t count = 0;

ISR (PCINT2_vect)
{
  state = (PIND & (1 << RX_PIN)) ? 0x1 : 0x0;

  TCCR0B = 0;
  count = TCNT0; 
  TCNT0 = 0;
  ovftemp = ovf;
  ovf = 0;
  //TCCR0B = (1<<CS02) | (1 << CS00); // start timer Prescaler 1024
  TCCR0B = (1<<CS02) | (1 << CS00); // start timer

}


ISR (TIMER0_OVF_vect)
{
  ovf += 1;
}


// Functions
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
  return (PIND & (1 << pin)) ? 1 : 0;
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

void pulse(uint8_t width)
{
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

void sendbyte(uint8_t byte)
{
  for(unsigned int i = 0; i < 8; i++) {
    writeBitC(HEAD_0_PIN, 0);
    writeBitC(HEAD_1_PIN, 0);
    _delay_us((1000000 * 5) / (2*BAUD));
  }

  writeBitC(HEAD_0_PIN, 1);
  writeBitC(HEAD_1_PIN, 1);
  _delay_us((1000000 * 5) / (2*BAUD));

    
  for(unsigned int i = 0; i < 8; i++) {

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

unsigned int converttickstonum(uint8_t num)
{
  if(num > 0 && num <= 4) {
    return 0;
  } else if(num > 4 && num <= 12) {
    return 1;
  } else if(num > 12 && num <= 20) {
    return 2;
  } else if(num > 20 && num <= 28) {
    return 3;
  } else if(num > 28 && num <= 38) {
    return 4;
  } else if(num > 38 && num <= 45) {
    return 5;
  } else if(num > 45 && num <= 55) {
    return 6;
  } else if(num > 55 && num <= 64) { // 59
    return 7;
  } else if(num > 64 && num <= 71) { // 67
    return 8;
  } else if(num > 71 && num <= 80) { // 75
    return 9;
  } else if(num > 80 && num <= 88) { // 84
    return 10;
  } else if(num > 88 && num <= 95) { // 92
    return 11;
  } else if(num > 95 && num <= 105) { // 101
    return 12;
  } else if(num > 105 && num <= 115) { // 109
    return 13;
  }
  
  return 0x0;
}



/*
uint8_t getpidbytefromheader(unsigned int ci)
{
  
  uint32_t temp = 0;
  unsigned int pos = 0;
  for(unsigned int i = 2; i < ci; i++) {
    unsigned int num = converttickstonum(capture[i]);
    for(unsigned int j = 0; j < num; j++) {
      if (i % 2 != 0) { //values[i] == 1
	temp |= 0x00000001 << pos;
      }
      pos++;
    }
  }

  temp &= 0x07F800;
  //  std::cout << "<->getpid" << std::endl;
  return (uint8_t)(temp >> 11);
  }*/

 /*

   uint8_t getpidbytefromheader(unsigned int ci)
   {
  
   uint16_t templ = 0;
   uint16_t temph = 0;  
   unsigned int pos = 0;
   for(unsigned int i = 2; i < ci; i++) {
   unsigned int num = converttickstonum(capture[i]);
   for(unsigned int j = 0; j < num; j++) {
   if (i % 2 != 0) { //values[i] == 1
   if (pos >= 16) {
   temph |= 0x0001 << (pos-16);
   }
   if (pos < 16) {
   templ |= 0x0001 << pos;        
   }
   }
   pos++;
   }
   }

   templ &= 0xF800;
   temph &= 0x0007;
   templ >> 11;
   //temph >> 11;
   //  std::cout << "<->getpid" << std::endl;
   return (uint8_t)(templ | temph);
   }
 */

uint8_t getpidbytefromheader(unsigned int ci)
{
  
  uint16_t templ = 0;
  uint16_t temph = 0;  
  unsigned int pos = 0;
  for(unsigned int i = 2; i < ci; i++) {
    unsigned int num = converttickstonum(capture[i]);
    for(unsigned int j = 0; j < num; j++) {
      if (i % 2 != 0) {
	if (pos < 32 && pos >= 16) {
	  temph |= 0x0001 << (pos-16);
	}
	if (pos < 16) {
	  templ |= 0x0001 << pos;        
	}
      }
      pos++;
    }
  }

  templ &= 0xF800;
  temph &= 0x0007;
  templ >>= 11;
  temph <<= 5;
  
  //temph >> 11;
  //  std::cout << "<->getpid" << std::endl;
  return (uint8_t)(templ | temph);
}



uint8_t getsyncbytefromheader(unsigned int ci)
{
  uint16_t temp = 0;
  unsigned int pos = 0;
  for(unsigned int i = 2; i < ci; i++) {
    unsigned int num = converttickstonum(capture[i]);
    for(unsigned int j = 0; j < num; j++) {
      if (i % 2 != 0) {
	if (pos < 16) {
	  temp |= 0x0001 << pos;
	}
      }
      pos++;
    }
  }

  temp &= 0x01FE;
  //std::cout << "<->getsync" << std::endl;
  return (uint8_t)(temp >> 1);
}

uint8_t getbytefrommessage(unsigned int n, unsigned int ci)
{
  
  uint16_t temp0 = 0;
  uint16_t temp1 = 0;
  uint16_t temp2 = 0;
  uint16_t temp3 = 0;
  uint16_t temp4 = 0;
  uint16_t temp5 = 0;
  uint16_t temp6 = 0;
  uint16_t temp7 = 0;
  
  unsigned int pos = 0;
  for(unsigned int i = 2; i < ci; i++) {
    unsigned int num = converttickstonum(capture[i]);
    for(unsigned int j = 0; j < num; j++) {
      if (i % 2 != 0) {
	if (pos < 32 && pos >= 16) {
	  temp0 |= 0x0001 << (pos-16);
	}
	if (pos < 48 && pos >= 32) {
	  temp1 |= 0x0001 << (pos-32);
	}
	if (pos < 64 && pos >= 48) {
	  temp2 |= 0x0001 << (pos-48);
	}
	if (pos < 80 && pos >= 64) {
	  temp3 |= 0x0001 << (pos-64);
	}
	if (pos < 96 && pos >= 80) {
	  temp4 |= 0x0001 << (pos-80);
	}
	if (pos < 112 && pos >= 96) {
	  temp5 |= 0x0001 << (pos-96);
	}
	if (pos < 128 && pos >= 112) {
	  temp6 |= 0x0001 << (pos-112);
	}
	if (pos < 146 && pos >= 128) {
	  temp7 |= 0x0001 << (pos-128);
	}
	
      }
      pos++;
    }
  }

  if (n == 0) {
    temp0 &= 0x1FE0;
    return (uint8_t) (temp0 >> 5);
  }
  if (n == 1) {
    temp0 &= 0x8000;
    temp1 &= 0x007F;
    return (uint8_t)(temp0 >> 15 | temp1 << 1);
  }
  if (n == 2) {
    temp1 &= 0xFE00;
    temp2 &= 0x0001;
    return (uint8_t) (temp1 >> 9 | temp2 << 7);
  }
  if (n == 3) {
    temp2 &= 0x07F8;
    return (uint8_t) (temp2 >> 3);
  }
  if (n == 4) {
    temp2 &= 0xE000;
    temp3 &= 0x001F;
    return (uint8_t) (temp2 >> 13 | temp3 << 3);
  }
  if (n == 5) {
    temp3 &= 0x3F80;
    return (uint8_t) (temp3 >> 7);
  }
  if (n == 6) {
    temp4 &= 0x01FE;
    return (uint8_t) (temp4 >> 1);
  }
  if (n == 7) {
    temp4 &= 0xF800;
    temp5 &= 0x0007;
    return (uint8_t) (temp4 >> 11 | temp5 << 5);
  }
  if (n == 8) {
    temp5 &= 0x1FE0;
    return (uint8_t) (temp5 >> 5);
  }
  if (n == 9) {
    temp5 &= 0x8000;
    temp6 &= 0x007F;
    return (uint8_t) (temp5 >> 15 | temp6 << 1);
  }
  
  return (uint8_t)0;
}



int main() {

  
  slave_init();
  
  uint8_t currentstate = 0;
  
  //float factor = 0;
  unsigned int ci = 0;

  
  for (unsigned int i = 0; i < bufsize; i++) {
    capture[i] = 0x0;
  }
  
  while(1) {

    if(ci >= bufsize) {
      while(1) {
	
	led(RED_LED_PIN, 1);
	_delay_us(100000);
	led(RED_LED_PIN, 0);
	_delay_us(100000);
	
      }
      
    }
    
    if(currentstate == 0 && state == 0x1) {

      currentstate = 1;
      
      //for (unsigned int i = 0; i < ovftemp; i++) {

      //pulse(1);
      //}

      //sendbyte(count);

      if (count > 100) {
	ci = 0;
	//factor = 13 / count;
      }
      
      capture[ci] = count;
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
      ci++;
    }
    if (currentstate == 1 && state == 0x1 && ci > 1) {
      uint16_t temp = 0;
      for (unsigned int i = 0; i < ci; i++) {
	temp += capture[i];
      }
      //if (ovf == 1) {
      if (temp > 270) {

	/*sendbyte(capture[12]);
	sendbyte(capture[13]);
	sendbyte(capture[14]);
	sendbyte(capture[15]);*/

	//sendbyte(ci);

	uint8_t pid = getpidbytefromheader(ci);
	uint8_t sync = getsyncbytefromheader(ci);

	
	if(sync == 0x55 && pid == 0x11) {

	  led(GREEN_LED_PIN, 0);

	}
	if(sync == 0x55 && pid == 0x12) {

	  led(GREEN_LED_PIN, 1);

	}
	/*if(sync == 0x55 && pid == 0x13) {

	  led(BLUE_LED_PIN, 0);

	}
	if(sync == 0x55 && pid == 0x14) {

	  led(BLUE_LED_PIN, 1);

	  }*/


	
	if (temp > 355) {

	  uint8_t mess0 = getbytefrommessage(0, ci);
	  sendbyte(mess0);
	  //sendbyte((uint8_t)capture[ci-1]);
	  //sendbyte((uint8_t)capture[ci-2]);
	  //sendbyte((uint8_t)capture[ci-3]);
	  //sendbyte((uint8_t)capture[ci-4]);
	  ci = 0;
	}
	  
	//sendbyte(pid);
	//sendbyte(sync);

	
	//Sendbyte((uint8_t)ci);
	//sendbyte((uint8_t)capture[12]);
	//sendbyte((uint8_t)capture[13]);
	//sendbyte((uint8_t)capture[14]);
	//sendbyte((uint8_t)capture[15]);
	//sendbyte((uint8_t)capture[16]);
	//sendbyte((uint8_t)capture[17]);
	
	//sendbyte((uint8_t)(0xFF & temp));
	//sendbyte((uint8_t)((0xFF00 & temp) >> 8));
   
	//sendbyte(numtics);
	
      }
    }
  }
    
  return 0;
}
