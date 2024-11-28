#include <avr/io.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <util/delay.h>

// LED Pins
#define BLUE_LED_PIN PB0
#define GREEN_LED_PIN PB1
#define RED_LED_PIN PB2

// Read Transmit Pins
#define RX_PIN PD0
#define TX_PIN PD1
#define CS_PIN PD4

// Timing
#define F_CPU 16000000UL // CPU-Frequenz des ATmega328P (16 MHz)
uint16_t BAUD = 19200;
uint32_t INTERVAL = 1000000*9/BAUD

int slave_init();
void writeBit(uint8_t, uint8_t);
uint8_t readBit(uint8_t);
uint8_t readBitWithOversampling(uint8_t, uint16_t);
uint8_t calculate_checksum(uint8_t, uint8_t);
void led(uint8_t, uint8_t);

// Functions
int slave_init() {
    // LED Init
    DDRB |= (1 << BLUE_LED_PIN); //OUT
    DDRB |= (1 << GREEN_LED_PIN); //OUT
    DDRB |= (1 << RED_LED_PIN); //OUT

    // COM Pin Init
    DDRD &= ~(1 << RX_PIN); //IN
    DDRD |= (1 << TX_PIN); //OUT
    DDRD |= (1 << CS_PIN); //OUT

    PORTD |= (1 << CS_PIN); //HIGH
    return 0;
}

void writeBit(uint8_t pin, uint8_t value) {
    if (value) {
        PORTD |= (1 << pin); // Write 1
    } else {
        PORTD &= ~(1 << pin); // Write 0
    }
}

uint8_t readBit(uint8_t pin) {
    return (PIND & (1 << pin)) ? 1 : 0;
}

uint8_t readBitWithOversampling(uint8_t pin, uint16_t oversample_rate) {
    if (oversample_rate == 0) return 0;

    uint16_t high_count = 0;
    uint16_t low_count = 0;

    for (uint16_t i = 0; i < oversample_rate; i++) {
        if (readBit(pin)) {
            high_count++;
        } else {
            low_count++;
        }
        _delay_us(INTERVAL / oversample_rate);
    }
    return (high_count > low_count) ? 1 : 0;
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

int main() {
    slave_init();
    uint16_t result = 0xFFFF;
    while (1) {
        while (1) {
            uint8_t bit = readBit(RX_PIN);
            result <<= 1;
            result |= (bit & 0x01);
            _delay_us(INTERVAL);

            if ((result & 0x3FFF) == 0x0001) {
                // led(RED_LED_PIN, 1);
                // _delay_us(100000);
                // led(RED_LED_PIN, 0);
                // _delay_us(100000);
                break;
            }
        }
        result = 0xFFFF;
        while (1) {
            uint8_t bit = readBit(RX_PIN);
            result <<= 1;
            result |= (bit & 0x01);
            _delay_us(INTERVAL);

            if ((result & 0x007F) == 0x0055) {
                led(RED_LED_PIN, 1);
                _delay_us(100000);
                led(RED_LED_PIN, 0);
                _delay_us(100000);
                break;
            }
        }
    }
    return 0;
}
