import serial
import time
import RPi.GPIO as GPIO

# GPIO für WAKE
WAKE_PIN = 23
GPIO.setmode(GPIO.BCM)
GPIO.setup(WAKE_PIN, GPIO.OUT)
GPIO.output(WAKE_PIN, GPIO.HIGH)  # WAKE aktivieren

# UART für LIN initialisieren
ser = serial.Serial(
    port='/dev/serial0',
    baudrate=19200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

def send_lin_frame(identifier, data):
    """
    Sendet einen vollständigen LIN-Frame: Break, Sync, Identifier, Daten, Checksumme.
    :param identifier: Message-ID (6-Bit)
    :param data: Liste von Datenbytes (maximal 8)
    """
    # Senden eines Break-Signals (ca. 13-Bit Low-Pegel)
    ser.break_condition = True
    time.sleep(0.013)  # 13 ms (entspricht Break)
    ser.break_condition = False
    time.sleep(0.001)  # 1 ms Pause nach Break

    # Sync Byte
    sync_byte = 0x55
    ser.write(bytes([sync_byte]))

    # Identifier (6-Bit, Paritätsbits hinzufügen)
    identifier_with_parity = identifier & 0x3F
    ser.write(bytes([identifier_with_parity]))

    # Datenfeld und Checksumme
    if data:
        checksum = calculate_checksum(identifier, data)
        ser.write(bytes(data) + bytes([checksum]))
    print(f"Gesendet: ID={identifier:#02x}, Daten={data}")

def calculate_checksum(identifier, data):
    """
    Berechnet die LIN-Checksumme (Classic Mode, nur Datenbytes).
    :param identifier: LIN-Identifier (6 Bit)
    :param data: Datenbytes
    :return: Berechnete Checksumme
    """
    checksum = sum(data) & 0xFF
    return checksum

def receive_lin_response():
    """
    Liest die Antwort des Slaves vom LIN-Bus.
    Erwartet 3 Bytes: Low Byte, High Byte und Checksumme.
    """
    response = ser.read(3)  # Versuche, 3 Bytes zu lesen (Timeout = 1 Sekunde)
    if len(response) == 3:
        responseLow, responseHigh, checksum = response
        responseData = (responseHigh << 8) | responseLow
        print(f"Empfangene Antwort: Daten=[{responseLow}, {responseHigh}], Checksumme={checksum:#02x}")
    else:
        print("Keine Antwort vom Slave erhalten.")

try:
    # Beispiel: LIN-Frame mit ID=1 und Daten [0x10, 0x20]
    send_lin_frame(0x01, [0x10, 0x20])
    time.sleep(0.1)  # Kurze Pause, um auf die Slave-Antwort zu warten
    receive_lin_response()
    # Kurze Pause vor dem nächsten Senden
    time.sleep(0.1)

except Exception as e:
    print(f"Fehler: {e}")
finally:
    GPIO.cleanup()
    ser.close()
