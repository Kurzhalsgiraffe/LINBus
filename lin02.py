import serial
import time
import RPi.GPIO as GPIO

# SLP-Pin für den LIN-Transceiver (MCP2003) konfigurieren
SLP_PIN = 23  # GPIO16
GPIO.setmode(GPIO.BCM)
GPIO.setup(SLP_PIN, GPIO.OUT)
GPIO.output(SLP_PIN, GPIO.HIGH)  # LIN-Transceiver aktivieren

# UART Schnittstelle konfigurieren
ser = serial.Serial(
    port='/dev/serial0',  # Standard UART des Raspberry Pi
    baudrate=19200,       # LIN-Bus Baudrate
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=2             # Timeout für Empfang erhöht
)

# Funktion zum Senden eines LIN-Frames
def send_lin_frame(message_id, data):
    """ Sendet einen LIN-Frame mit dem angegebenen Dateninhalt """
    # Break-Signal für LIN (entsprechend der Spezifikation)
    ser.write(b'\x00' * 20)  # Längeres Break-Signal
    time.sleep(0.02)  # Kleine Pause

    # Sync-Byte und ID (Beispiel-ID, kann angepasst werden)
    sync_byte = b'\x55'

    # LIN-Frame zusammensetzen: Sync-Byte + ID + Daten + Checksumme
    data_bytes = data.to_bytes(2, 'little')  # Konvertiere die Zahl in zwei Bytes
    checksum = (sum(data_bytes) + int.from_bytes(message_id, 'little')) & 0xFF
    frame = sync_byte + message_id + data_bytes + checksum.to_bytes(1, 'little')

    # Frame senden
    ser.write(frame)
    print(f"Gesendeter Frame: {frame.hex()}")

# Funktion zum Empfangen einer Antwort vom Slave
def receive_lin_response():
    """ Empfängt eine Antwort vom Slave """
    response = ser.read(5)  # Wir erwarten 5 Bytes
    if len(response) == 5:
        print(f"Rohdaten empfangen: {response.hex()}")
        sync_byte = response[0]
        message_id = response[1]
        data_bytes = response[2:4]
        received_checksum = response[4]

        calculated_checksum = (sum(data_bytes) + message_id) & 0xFF

        received_data = int.from_bytes(data_bytes, 'little')

        if received_checksum != calculated_checksum:
            print(f"Fehlerhafte Checksumme: erhalten {received_checksum:02X}, erwartet {calculated_checksum:02X}")
            return None
        return received_data
    else:
        print(f"Keine Antwort oder unvollständige Antwort empfangen: '{response}'")
        return None


if __name__ == "__main__":
    try:
        # Sende- und Empfangsprozess
        # 1000 = Blau
        # 1001 = Grün
        # 1002 = Rot
        values_to_send = [1000]  # Werte, die nacheinander gesendet werden
        expected_responses = [1001]  # Erwartete Antworten von den Slaves

        # Durchläuft die Slaves nacheinander
        message_id = b'\x01'
        while True:
            for i in range(len(values_to_send)):
                # Nachricht an den aktuellen Slave senden
                print(f"Sende Wert: {values_to_send[i]}")
                send_lin_frame(message_id, values_to_send[i])
                time.sleep(1)

                # Antwort vom Slave empfangen und überprüfen
                received_data = receive_lin_response()
                if received_data is not None:
                    print(f"Empfangene Antwort: {received_data}")
                    print(f"Match: {received_data == expected_responses[i]}")
                else:
                    print("Keine gültige Antwort erhalten.")
            time.sleep(5)

    except KeyboardInterrupt:
        print("Programm abgebrochen.")

    finally:
        # Aufräumen
        GPIO.output(SLP_PIN, GPIO.LOW)  # LIN-Transceiver deaktivieren
        GPIO.cleanup()
        ser.close()
