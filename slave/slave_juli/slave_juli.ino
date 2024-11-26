
// AVR Serial Reception @ 230.4k bps
// AVR Receives character 'A' Blinks the LED  on PC4
//                        'B' Beeps  the Buzzer on PC5
// External Oscillator = 11.0592MHz
// Controller ATmega328p


//+------------------------------------------------------------------------------------------------+
//| Compiler           : AVR GCC (WinAVR)                                                          |
//| Microcontroller    : ATmega328p                                                                |
//| Programmer         : Rahul.Sreedharan                                                          |
//| Date               : 16-July-2019                                                              |
//+------------------------------------------------------------------------------------------------+

//(C) www.xanthium.in 
// Visit to Learn More 


#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

// +-----------------------------------------------------------------------+ //
// | ATmega328p Baudrate values for UBRRn for external crystal 11.0592MHz  | //
// +-----------------------------------------------------------------------+ //

#define BAUD_RATE_4800_BPS  143 // 4800bps
#define BAUD_RATE_9600_BPS  71  // 9600bps

#define BAUD_RATE_14400_BPS  47 // 14.4k bps
#define BAUD_RATE_19200_BPS  35 // 19.2k bps  
#define BAUD_RATE_28800_BPS  23 // 28.8k bps
#define BAUD_RATE_38400_BPS  17 // 38.4k bps
#define BAUD_RATE_57600_BPS  11 // 57.6k bps
#define BAUD_RATE_76800_BPS   8 // 76.8k bps

#define BAUD_RATE_115200_BPS  5 // 115.2k bps
#define BAUD_RATE_230400_BPS  2 // 230.4k bps
#define F_CPU 16000000UL // CPU-Frequenz des ATmega328P (16 MHz)

// LED PINS
#define BLUE_LED_PIN PB0
#define GREEN_LED_PIN PB1
#define RED_LED_PIN PB2



#define ASYNCHRONOUS (0 << UMSEL01) | (0 << UMSEL00) // Set by default

// +-----------------------------------------------------------------------+ //

// Funktion, um eine LED blinken zu lassen
void blink_led(int pin, int count) {
    DDRB |= (1 << pin); // Pin als Ausgang setzen
    for (int i = 0; i < count; i++) {
        PORTB |= (1 << pin);  // LED einschalten
        _delay_ms(200);       // 200 ms an
        PORTB &= ~(1 << pin); // LED ausschalten
        _delay_ms(200);       // 200 ms aus
    }
}


int main()
{
	
	unsigned int ubrr = ((F_CPU)/(16 * 19200)) - 1;
	
	// PORTC = 0x00; //All LED's OFF
	// PORTD = 0x00;
	
	/* Set Baudrate @ 230.4k bps */
	UBRR0H = (ubrr>>8);
	UBRR0L = (ubrr);
  //UCSR0C = ASYNCHRONOUS
	
	/*Enable receiver  */
	UCSR0B = (1<<RXEN0);
	
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = 0b00000110;
	//UCSR0C = 0x06;
  //UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);




	while(1)
	{
		while ( !(UCSR0A & (1<<RXC0)) ); /* Wait for data to be received */
	
		switch(UDR0) // using the switch() statement to  control the LED and Buzzer		
		{

			case 'A' : 
        blink_led(BLUE_LED_PIN, 1);  // Blaue LED blinkt 1-mal
        break;
    
      case 'B' : 
        blink_led(GREEN_LED_PIN, 1); // Grüne LED blinkt 1-mal
        break;			
						
			default  :
        break;
		}//end of Switch()
	}//end of While(1)
}//end of main
	
	
	
	
	
