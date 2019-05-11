import serial


ser = serial.Serial(

  port='/dev/ttyUSB0',
  baudrate = 9600,
  parity = serial.PARITY_NONE,
  stopbits=serial.STOPBITS_ONE,
  bytesize=serial.EIGHTBITS,
  timeout=1)

while True:
  ser.write('1\n'.encode('utf-8'))
  print(ser.readline())