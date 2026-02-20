import serial
import serial.tools.list_ports
import sys
import threading
import os

# ==============================
# Select Serial Port
# ==============================
ports = list(serial.tools.list_ports.comports())
if not ports:
    print("No serial ports found.")
    sys.exit()

print("Available Serial Ports:")
for i, port in enumerate(ports):
    print(f"{i}: {port.device}")

index = int(input("Select port number: "))
port_name = ports[index].device

ser = serial.Serial(port_name, 115200)
print(f"\nConnected to {port_name}")
print("Start typing... (Press ESC to quit)\n")


# ==============================
# Windows Version
# ==============================
if os.name == 'nt':
    import msvcrt

    while True:
        if msvcrt.kbhit():
            key = msvcrt.getch()

            # ESC to exit
            if key == b'\x1b':
                break
            
            # Arrow keys on Windows return b'\xe0' first
            # if key == b'\xe0':
            #     arrow = msvcrt.getch()  # Get second byte
            #     if arrow == b'H':   # Up
            #         ser.write(b'\x1b[A')
            #     elif arrow == b'P': # Down
            #         ser.write(b'\x1b[B')
            #     elif arrow == b'M': # Right
            #         ser.write(b'\x1b[C')
            #     elif arrow == b'K': # Left
            #         ser.write(b'\x1b[D')

            # Backspace
            if key == b'\x08':
                ser.write(b'\x08')
            else:
                ser.write(key)

# ==============================
# macOS / Linux Version
# ==============================
else:
    import tty
    import termios

    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)

    try:
        tty.setraw(sys.stdin.fileno())

        while True:
            key = sys.stdin.read(1)

            # ESC to exit
            if key == '\x1b':
                break

            # Backspace (Mac sends \x7f)
            if key == '\x7f':
                ser.write(b'\x08')
            else:
                ser.write(key.encode())

    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)

ser.close()
print("\nDisconnected.")