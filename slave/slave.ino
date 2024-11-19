#define F_CPU 16000000UL // CPU-Frequenz des ATmega328P (16 MHz)
#include <avr/io.h>
#include <util/delay.h>

// Definition der LED-Pins
#define BLUE_LED_PIN PB0
#define GREEN_LED_PIN PB1
#define RED_LED_PIN PB2

// Initialisierung der UART für LIN-Kommunikation
void uart_init() {
    // Baudrate auf 19200 einstellen
    uint16_t ubrr_value = F_CPU / 16 / 19200 - 1;
    UBRR0H = (ubrr_value >> 8);
    UBRR0L = ubrr_value;

    // UART: RX und TX aktivieren, 8 Datenbits, keine Parität, 1 Stoppbit
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Empfang eines Bytes über UART
uint8_t uart_receive_byte() {
    while (!(UCSR0A & (1 << RXC0))) {
        // Warten, bis Daten empfangen wurden
    }
    return UDR0; // Empfangene Daten zurückgeben
}

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

int main() {
    // UART initialisieren
    uart_init();

    // Hauptschleife
    while (1) {
        // LIN-Frame (5 Bytes) empfangen
        uint8_t sync_byte = uart_receive_byte();  // Sync-Byte (meist 0x55)
        uint8_t message_id = uart_receive_byte(); // Nachrichten-ID
        uint8_t data_low = uart_receive_byte();   // Niedriges Datenbyte
        uint8_t data_high = uart_receive_byte();  // Hohes Datenbyte
        uint8_t checksum = uart_receive_byte();   // Checksumme

        // Überprüfen, ob Sync-Byte korrekt ist
        if (sync_byte != 0x55) {
            continue; // Ungültiges LIN-Frame, überspringen
        }

        // Zusammengebauter Datenwert
        uint16_t received_data = ((uint16_t)data_high << 8) | data_low;

        // Datenverarbeitung: LEDs entsprechend blinken lassen
        // switch (message_id) {
        // case 0x01:
        //     blink_led(BLUE_LED_PIN, 1);  // Blaue LED blinkt 1-mal
        //     break;
        // case 0x02:
        //     blink_led(GREEN_LED_PIN, 2); // Grüne LED blinkt 2-mal
        //     break;
        // case 0x03:
        //     blink_led(RED_LED_PIN, 3);   // Rote LED blinkt 3-mal
        //     break;
        // default:
        //     // Ungültige oder unbekannte Nachricht
        //     blink_led(RED_LED_PIN, 5);  // Fehleranzeige: Rote LED blinkt 5-mal
        //     break;
        // }

        // Datenverarbeitung: LEDs entsprechend blinken lassen
        if (received_data == 1000) {
            blink_led(BLUE_LED_PIN, 1);  // Blaue LED blinkt 1-mal
        } else if (received_data == 1001) {
            blink_led(GREEN_LED_PIN, 1); // Grüne LED blinkt 1-mal
        } else if (received_data == 1002) {
            blink_led(RED_LED_PIN, 1);   // Rote LED blinkt 1-mal
        } else {
            // Für andere Werte keine LEDs blinken
            // Optional könnte man hier eine Fehlermeldung anzeigen
            blink_led(RED_LED_PIN, 3);   // Rote LED blinkt 3-mal als Fehleranzeige
        }

        // Debug-LED: Daten empfangen (optional für Testzwecke)
        // blink_led(GREEN_LED_PIN, data_low); // Datenwert als Blinkanzahl (nur low byte)
    }

    return 0;
}
