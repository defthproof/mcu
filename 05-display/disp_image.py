from PIL import Image
import serial
import time

PORT = "COM15"
BAUDRATE = 115200
IMAGE_PATH = r"C:\build\05-display\jews.png"

DISPLAY_WIDTH = 320
DISPLAY_HEIGHT = 240


def main():
    image = Image.open(IMAGE_PATH).convert("RGB")
    width, height = image.size

    print(f"Image size: {width}x{height}")

    if width != DISPLAY_WIDTH or height != DISPLAY_HEIGHT:
        print(f"Error: image must be exactly {DISPLAY_WIDTH}x{DISPLAY_HEIGHT}")
        return

    ser = serial.Serial(port=PORT, baudrate=BAUDRATE, timeout=1.0)

    try:
        print(f"Port {ser.name} opened")

        ser.reset_input_buffer()
        ser.write(b"disp_screen 000000\n")
        time.sleep(0.1)

        for y in range(height):
            for x in range(width):
                r, g, b = image.getpixel((x, y))
                command = f"disp_px {x} {y} {r:02X}{g:02X}{b:02X}\n"
                ser.write(command.encode("ascii"))

            if y % 10 == 0:
                print(f"Row {y}/{height}")

        print("Image sent")

    finally:
        time.sleep(0.1)
        ser.close()
        print("Port closed")


if __name__ == "__main__":
    main()