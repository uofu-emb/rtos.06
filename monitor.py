import serial
with serial.Serial("/dev/tty.usbmodem141301") as s:
    l = s.readline()
    while l:
        print(l.decode("utf-8"), end="")
        l = s.readline()
