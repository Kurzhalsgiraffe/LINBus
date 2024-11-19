#include <SoftwareSerial.h>
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>


const int CS_PIN = 4;      // PD4, Kontrolliert den CS-Pin des MCP2003
const byte SLAVE_ID = 0x01; // ID des Slaves ist 1 (Muss für die anderen immer um 1 erhöht werden!)

// Globale Variable zum Speichern der empfangenen Nachricht (-1 bedeutet keine Nachricht)
int receivedData = -1;

// UART-Konfiguration für den LIN-Bus
SoftwareSerial linSerial(0, 1);  // RX, TX Pins des Arduino Nano

void setup() {
  // Initialisierung der LIN-Bus-Software-Schnittstelle
  linSerial.begin(19200);  // Baudrate für LIN-Bus

  // Konfiguration des CS-Pins
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);  // LIN-Transceiver aktivieren
}

void blink() {
  DDRB |= (1<<DDB1);
  PORTB |= (1<<PORTB1);
  _delay_ms(50);
  PORTB &= ~ (1<<PORTB1);
  _delay_ms(50);
}

void loop() {
  blink();
  // Überprüfen, ob Daten vom LIN-Bus empfangen wurden
  if (linSerial.available() >= 4) {
    // Empfangene Daten auslesen (4 Bytes erwartet)
    byte syncByte = linSerial.read();     // Synchronsignal // Eventuell muss das hier an sync_byte = b'\x55' aus dem Master Programm angepasst werden, das müssen wir testen
    byte messageId = linSerial.read();    // Message ID

    // Nur Nachrichten mit der ID 1 verarbeiten
    if (messageId == SLAVE_ID) {
      byte dataLow = linSerial.read();    // Daten-Byte (niedriges Byte)
      byte dataHigh = linSerial.read();   // Daten-Byte (hohes Byte)

      // Empfangene Daten zusammenfügen und in der globalen Variable speichern
      receivedData = (dataHigh << 8) | dataLow;

      // Antwort berechnen (erhöht um 1)
      int responseData = receivedData + 1;
      byte responseLow = responseData & 0xFF;         // Niedriges Byte der Antwort
      byte responseHigh = (responseData >> 8) & 0xFF; // Hohes Byte der Antwort

      // Checksumme berechnen
      byte checksum = (messageId + responseLow + responseHigh) & 0xFF;

      // Antwort als LIN-Frame senden
      linSerial.write(responseLow);     // Niedriges Daten-Byte
      linSerial.write(responseHigh);    // Hohes Daten-Byte
      linSerial.write(checksum);        // Checksumme
    }
  }

  delay(50); // Kurze Verzögerung
}