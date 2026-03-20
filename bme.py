import time
import serial
import matplotlib.pyplot as plt


def read_float(ser):
    while True:
        try:
            line = ser.readline().decode("ascii").strip()
            value = float(line)
            return value
        except ValueError:
            continue


def main():
    ser = serial.Serial(port="COM15", baudrate=115200, timeout=0.0)

    if ser.is_open:
        print(f"Port {ser.name} opened")
    else:
        print(f"Port {ser.name} closed")
        return

    measure_ts = []
    measure_temp_C = []
    measure_pres_Pa = []
    measure_hum_pct = []

    start_ts = time.time()

    try:
        while True:
            ts = time.time() - start_ts

            ser.write("temp\n".encode("ascii"))
            temp_C = read_float(ser)

            ser.write("pres\n".encode("ascii"))
            pres_Pa = read_float(ser)

            ser.write("hum\n".encode("ascii"))
            hum_pct = read_float(ser)

            measure_ts.append(ts)
            measure_temp_C.append(temp_C)
            measure_pres_Pa.append(pres_Pa)
            measure_hum_pct.append(hum_pct)

            print(f"{temp_C:.2f} C | {pres_Pa:.2f} Pa | {hum_pct:.2f} % | {ts:.2f} s")

            time.sleep(0.2)

    except KeyboardInterrupt:
        print("Measurement stopped by user")

    finally:
        ser.close()
        print("Port closed")

        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(measure_ts, measure_temp_C)
        plt.title("График температуры от времени")
        plt.xlabel("время, с")
        plt.ylabel("температура, C")

        plt.subplot(3, 1, 2)
        plt.plot(measure_ts, measure_pres_Pa)
        plt.title("График давления от времени")
        plt.xlabel("время, с")
        plt.ylabel("давление, Па")

        plt.subplot(3, 1, 3)
        plt.plot(measure_ts, measure_hum_pct)
        plt.title("График влажности от времени")
        plt.xlabel("время, с")
        plt.ylabel("влажность, %")

        plt.tight_layout()
        plt.show()


if __name__ == "__main__":
    main()