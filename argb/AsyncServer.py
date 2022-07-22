from asyncio import Protocol, get_event_loop
from serial_asyncio import create_serial_connection
from messages_pb2 import Request
from traceback import format_exc
from .stream import PacketProcessor

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
        self.packet = PacketProcessor()
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

    def process_packet(self, data):
        result = self.packet.process(data)
        if result is None:
            return
        t, message = result
        if t == 'Response':
            if message.HasField('log') and message.log.id == 9:
                try:
                    self.delegate.ready(self)
                except:
                    print(format_exc())
            else:
                try:
                    should_stop = self.delegate.process(self, message)
                except:
                    print(format_exc())
                if should_stop:
                    self.transport.close()
        elif t == 'DebugMessage':
            try:
                self.delegate.debug_message(self, message)
            except:
                print(format_exc())
        else:
            # error, exception
            print(t, message)

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
                if len(buffer) > 0:
                    self.incoming += 1
                    if len(buffer) <= 4:
                        self.log('detected message without a crc')
                    else:
                        self.process_packet(buffer)
        if drop_up_to is not None:
            self.input_buffer = self.input_buffer[drop_up_to:]

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
        encoded_data = self.packet.encode(msg)
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

