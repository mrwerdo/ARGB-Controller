from serial import Serial
from cobs.cobs import encode, decode
from time import sleep
from pycrc.algorithms import Crc
from messages_pb2 import *
from textwrap import dedent

class Connection:
    def __init__(self, port, baudrate=9600):
        self.serialPort = Serial()
        self.serialPort.baudrate = baudrate
        self.serialPort.port = port
        self.serialPort.timeout = 1
        self.serialPort.write_timeout = 0
        self.crc = Crc(
            width=32,
            poly=0x04c11db7,
            reflect_in=True,
            xor_in=0xffffffff,
            xor_out=0xffffffff,
            reflect_out=True
        )
        self.logging_enabled = False
    
    def __enter__(self):
        self.serialPort.__enter__()
        sleep(2)

    def __exit__(self, *args):
        self.serialPort.__exit__(*args)
    
    def log(self, *args, **kwargs):
        if self.logging_enabled:
            print(*args, **kwargs)

    def receiveMessage(self):
        data = self.receive()
        message = Response()
        message.ParseFromString(data)
        return message

    
    def receive(self):
        buffer = bytearray()
        while True:
            if self.serialPort.in_waiting > 0:
                byte = self.serialPort.read()[0]
                if byte == 0:
                    if len(buffer) > 0:
                        break
                else:
                    buffer.append(byte)
        if len(buffer) <= 4:
            print('received too short a buffer')
            return None
        else:
            try:
                msg = decode(buffer)
                received_crc = msg[-4:]
                crc = self.crc.bit_by_bit(msg[:-4])
                crc = bytes([
                    (crc & 0x000000FF) >> 0*8,
                    (crc & 0x0000FF00) >> 1*8,
                    (crc & 0x00FF0000) >> 2*8,
                    (crc & 0xFF000000) >> 3*8,
                ])
                if received_crc != crc:
                    print(f'warning: received crc: {received_crc}, expected crc: {crc}')
                self.log(dedent(f'''
                Received Message:
                    length: {len(buffer)}
                    buffer: {buffer.hex('-')}
                    data: {msg[:-4].hex('-')}
                    received crc: {received_crc.hex('-')}
                    calculated crc: {crc.hex('-')}
                '''))
                return msg[:-4]
            except Exception as error:
                print(error)
                return None

    def send(self, message):
        checksum = self.crc.bit_by_bit(message).to_bytes(4, byteorder='little')
        data = bytearray(message)
        data.extend(checksum)
        encoded_data = encode(data)
        self.log(dedent(f'''
        Sending Message:
            length: {len(encoded_data)}
            buffer: {encoded_data.hex('-')}
            data: {data[:-4].hex('-')}
            crc: {checksum.hex('-')}
        '''), end='')
        self.serialPort.write(encoded_data)
        self.serialPort.write([0])
        self.serialPort.flush()
        self.log('done')

    def sendMessage(self, msg):
        data = msg.SerializeToString()
        self.send(data)
