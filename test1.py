import serial
import time
import RPi.GPIO as GPIO

# GPIO-Pin für WAKE (Pin 6 des MCP2003)
WAKE_PIN = 23  # Laut deinem Schaltplan auf GPIO18

# GPIO initialisieren
GPIO.setmode(GPIO.BCM)
GPIO.setup(WAKE_PIN, GPIO.OUT)

# MCP2003 aktivieren
GPIO.output(WAKE_PIN, GPIO.HIGH)  # WAKE auf HIGH setzen
time.sleep(0.1)  # Zeit geben, damit der Transceiver hochfährt

# Serielle Verbindung konfigurieren
ser = serial.Serial(
    port='/dev/serial0',  # Standard-UART des Raspberry Pi
    baudrate=19200,       # LIN-typische Baudrate (19,2 kBit/s)
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1             # Timeout in Sekunden
)

# Funktion zum Senden einer LIN-Nachricht
def send_lin_message(identifier, data):
    """
    Sendet eine LIN-Nachricht mit Header und Daten.
    :param identifier: LIN-Identifier (6-Bit, z. B. 0x12)
    :param data: Liste der Datenbytes (max. 8 Bytes)
    """
    # Header (Break, Sync, Identifier) erstellen
    header = b'\x00' * 12 + b'\x55' + bytes([identifier & 0x3F])  # Break, Sync, Identifier (6 Bit)
    ser.write(header)  # Header senden

    # Datenfeld und Checksumme erstellen
    checksum = calculate_checksum(data)
    data_frame = bytes(data) + bytes([checksum])
    ser.write(data_frame)  # Datenfeld senden

    print(f"Gesendete Nachricht: ID={identifier:#04x}, Daten={data}, Checksumme={checksum:#04x}")

# Funktion zur Berechnung der Checksumme (Classic LIN-Checksum)
def calculate_checksum(data):
    """
    Berechnet die LIN-Checksumme (ohne Identifier).
    :param data: Datenbytes
    :return: Checksumme (1 Byte)
    """
    checksum = sum(data) & 0xFF  # Summe der Datenbytes auf 8 Bit begrenzen
    return checksum

try:
    # Beispiel: Eine LIN-Nachricht senden
    lin_id = 0x12  # Beispiel-Identifier (6 Bit)
    lin_data = [0x10, 0x20, 0x30]  # Beispiel-Daten
    while True:
        send_lin_message(lin_id, lin_data)
        time.sleep(0.1)

    print("LIN-Nachricht erfolgreich gesendet.")
finally:
    # GPIO aufräumen
    GPIO.cleanup()
    ser.close()
