from asyncio import Protocol, get_event_loop
from serial_asyncio import create_serial_connection
from cobs.cobs import encode, decode
from pycrc.algorithms import Crc
from messages_pb2 import Request, Response, DebugMessage
from traceback import format_exc

def pack_rgb(v):
    a, b, c = v
    return (a << 16) | (b << 8) | c

def pack_ahds(v):
    a, h, d, s = v
    return (a << 24) | (h << 16) | (d << 8) | s

class ARGBProtocol(Protocol):
    def __init__(self, *args, **kwargs):
        self.logging_enabled = True
        self.delegate = None
        self.input_buffer = bytearray()
        self.crc = Crc(
            width=32,
            poly=0x04c11db7,
            reflect_in=True,
            xor_in=0xffffffff,
            xor_out=0xffffffff,
            reflect_out=True
        )
        self.incoming = 0
        self.outgoing = 0
        super().__init__(*args, **kwargs)

    def set_delegate(self, delegate):
        self.delegate = delegate

    def log(self, msg):
        print(f'protocol: {msg}')

    def connection_made(self, transport):
        self.transport = transport
        self.transport.set_write_buffer_limits(80, 40)
        self.log('connection made')
    
    def connection_lost(self, exc):
        self.log('connection lost')
        self.delegate.completed(self)
        self.transport.loop.stop()

    def data_received(self, data):
        self.input_buffer.extend(data)
        self.detect_packets()
    
    def pause_writing(self):
        self.log('pause writing')

    def resume_writing(self):
        self.log('resume writing')

    def decode_protobuf(self, data):
        is_response = True
        try:
            message = Response()
            message.ParseFromString(data)
        except:
            is_response = False
            message = DebugMessage()
            message.ParseFromString(data)

        if is_response:
            if message.HasField('log') and message.log.id == 9:
                self.delegate.ready(self)
            else:
                should_stop = self.delegate.process(self, message)
                if should_stop:
                    self.transport.close()
        else:
            self.delegate.debug_message(self, message)

    def detect_packets(self):
        start_of_message = None
        drop_up_to = None
        for (index, element) in enumerate(self.input_buffer):
            if element != 0 and start_of_message is None:
                start_of_message = index
            elif element == 0 and start_of_message is not None:
                buffer = self.input_buffer[start_of_message:index]
                start_of_message = None
                drop_up_to = index
                try:
                    self.process_packet(buffer)
                except:
                    print(format_exc())
        if drop_up_to is not None:
            self.input_buffer = self.input_buffer[drop_up_to:]

    def process_packet(self, part):
        if len(part) > 0:
            self.incoming += 1
            if len(part) <= 4:
                self.log('detected message without a crc')
            else:
                try:
                    msg = decode(part)
                except Exception as error:
                    self.log(f'{error} incoming: {self.incoming}, outgoing: {self.outgoing}, buffer: {part.hex("-")}')
                    return

                data, crc = msg[:-4], msg[-4:]
                if self.check_crc(data, crc):
                    self.decode_protobuf(data)
    
    def check_crc(self, msg, received_crc):
        crc = self.crc.bit_by_bit(msg)
        crc = bytes([
            (crc & 0x000000FF) >> 0*8,
            (crc & 0x0000FF00) >> 1*8,
            (crc & 0x00FF0000) >> 2*8,
            (crc & 0xFF000000) >> 3*8,
        ])
        if received_crc != crc:
            self.log(f'warning: received crc: {received_crc.hex("-")}, expected crc: {crc.hex("-")}')
            return False
        return True

    def set_light(self,
            index,
            start,
            end,
            start_color,
            end_color,
            ahds,
            start_color_alt=None,
            end_color_alt=None):
        start_color_alt = start_color_alt or start_color
        end_color_alt = end_color_alt or end_color
        request = Request()
        message = request.set_light
        message.id = index
        message.range = (start << 16) | end
        message.start_color = pack_rgb(start_color)
        message.end_color = pack_rgb(end_color)
        message.start_color_alt = pack_rgb(start_color_alt)
        message.end_color_alt = pack_rgb(end_color_alt)
        message.ahds = pack_ahds(ahds)
        self.send_request(request)

    def send_request(self, msg):
        message = msg.SerializeToString()
        checksum = self.crc.bit_by_bit(message).to_bytes(4, byteorder='little')
        data = bytearray(message)
        data.extend(checksum)
        encoded_data = encode(data)
        self.transport.write(encoded_data)
        self.transport.write(b'\x00')
        self.outgoing += 1

    def commit(self, delta):
        request = Request()
        message = request.commit_transaction
        message.timestamp = delta
        self.send_request(request)


class AsyncServer:
    def __init__(self, device, delegate):
        self.delegate = delegate
        self.logging_enabled = False
        self.loop = get_event_loop()
        self.connection = create_serial_connection(
                self.loop, 
                ARGBProtocol, 
                device, 
                baudrate=9600)

    def log(self, msg):
        print(f'server: {msg}')

    def main(self):
        transport, protocol = self.loop.run_until_complete(self.connection)
        protocol.logging_enabled = self.logging_enabled
        protocol.set_delegate(self.delegate)
        try:
            self.loop.run_forever()
        except KeyboardInterrupt:
            print()
            transport.close()
            self.loop._run_once()
        self.loop.close()

