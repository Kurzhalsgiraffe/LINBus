x = 1000
bytes = x.to_bytes(2, "little")
summe = sum(bytes)
message_id = int.from_bytes(b'\x01', 'little')
checksum = summe + message_id & 0xFF
print(bytes)
print(summe)
print(message_id)
print(checksum.to_bytes(1, 'little'))
