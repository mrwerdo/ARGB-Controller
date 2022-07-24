from pycrc.algorithms import Crc
from cobs.cobs import encode, decode
from messages_pb2 import Request, Response, DebugMessage

class PacketProcessor:
    def __init__(self):
        self.crc = Crc(
            width=32,
            poly=0x04c11db7,
            reflect_in=True,
            xor_in=0xffffffff,
            xor_out=0xffffffff,
            reflect_out=True
        )

    def process(self, data):
        try:
            msg = decode(data)
        except Exception as error:
            print(f'{error}: {data}')
            return None
        protobuf_payload, crc = msg[:-4], msg[-4:]
        if not self._check_crc(protobuf_payload, crc):
            return None
        return self._decode_protobuf(protobuf_payload)

    def _decode_protobuf(self, data):
        try:
            message = Response()
            message.ParseFromString(data)
            return ('Response', message)
        except Exception as e:
            error = e
        try:
            message = DebugMessage()
            message.ParseFromString(data)
            return ('DebugMessage', message)
        except:
            pass

        return ('error', error)

    def _check_crc(self, msg, received_crc):
        crc = self.crc.bit_by_bit(msg)
        crc = bytes([
            (crc & 0x000000FF) >> 0*8,
            (crc & 0x0000FF00) >> 1*8,
            (crc & 0x00FF0000) >> 2*8,
            (crc & 0xFF000000) >> 3*8,
        ])
        result = received_crc == crc
        if not result:
            print(f'warning: received crc: {received_crc.hex("-")}, expected crc: {crc.hex("-")}')
        return result

    def encode(self, msg):
        message = msg.SerializeToString()
        checksum = self.crc.bit_by_bit(message).to_bytes(4, byteorder='little')
        data = bytearray(message)
        data.extend(checksum)
        return encode(data)
