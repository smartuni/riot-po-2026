#!/usr/bin/env -S uv run --script
#
# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "pyserial>=3.5",
# ]
# ///

import serial

s = serial.Serial(port="/dev/ttyACM0", baudrate=115200)

key_bundle = input("Provide provision base64 now:\n")

for i in range(0, len(key_bundle), 128):
    print(key_bundle[i:i+128])
    res = s.write(data=str.encode(key_bundle[i:i+128]))
    print("Written {} bytes\n".format(res))
