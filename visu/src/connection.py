import serial

class SerialConfig:
    '''Class holding the data neccessary for the serial configuration'''
    def __init__(self, port, baud, bytesize, parity, stopbits) -> None:
        '''Constructor'''
        self.port = port
        self.baud = baud
        self.bytesize = bytesize
        self.parity = parity
        self.stopbits = stopbits

serial_config = SerialConfig('COM5', 115200, 8, 'N', 1)
try:
    with serial.Serial(serial_config.port, serial_config.baud) as ser:
        # start char
        print('writing')
        ser.write(b'a')
        if ser.read() != b'a':
            print('no connection')
        else:
            print('serial connection available')
except Exception as e:
    print("no connection")