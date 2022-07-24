from serial_asyncio import open_serial_connection
from .stream import PacketProcessor
from asyncio import Queue, get_event_loop
from .messages import Response, Request, DebugMessage
from .messages import set_light as build_set_light, commit as build_commit

class DebugDelegate:
    def log(self, msg):
        print(f'debug: {msg}')

    def debug(self, msg):
        self.log(str(msg).replace('\n', ' '))

    def error(self):
        self.log('error')

class Device:
    def __init__(self, device, delegate=None):
        self.port = device
        self.reader = None
        self.writer = None
        self.delegate = delegate
        self.incoming = 0
        self.outgoing = 0
        self.packet = PacketProcessor()
        self.write_queue = Queue()
        self.logging = False

    def log(self, msg):
        if self.logging:
            print(f'device: {msg}')

    async def __aenter__(self):
        self.log('enter')
        self.reader, self.writer = await open_serial_connection(url=self.port, baudrate=9600)
        # Discard messages where the beginning has been missed.
        await self.reader.readuntil(separator=b'\x00')
        await self._wait_until_ready()
        return self

    async def __aexit__(self, exc_type, exc, tb):
        print(exc_type, exc, tb)
        self.log('exit')

    def _debug_message(self, message):
        self.log(f'debug message {message}')

    def _read_error():
        self.log('read error')

    async def _read_packet(self):
        #self.log('read packet')
        data = await self.reader.readuntil(separator=b'\x00')
        if data == b'\x00':
            # read a bogus end of packet.
            return None
        if len(data) <= 5:
            # data ends with the separator, and all messages
            # must contain a four byte crc.
            self.log(f'read data length: {len(data)}, data: {data.hex("-")}')
            return None
        result = self.packet.process(data[:-1])
        if result is None:
            self.log('read result is none')
            return None
        t, message = result
        if t == 'Response' or t == 'DebugMessage':
            return message
        else:
            # error, exception
            print(t, message)
            return None

    async def _wait_until_ready(self):
        self.log('wait until ready')
        while True:
            message = await self._read_packet()
            if message is None:
                continue
            if not isinstance(message, Response):
                continue
            if message.HasField('log') and message.log.id == 9:
                break
        self.log('ready')

    async def _read_non_debug_message(self):
        self.log('read non debug message')
        while True:
            message = await self._read_packet()
            if message is None:
                continue
            elif isinstance(message, Response):
                return message
            elif isinstance(message, DebugMessage):
                self._debug_message(message)
            else:
                self._read_error()

    async def write(self, message):
        self.log('write')
        data = self.packet.encode(message)
        self.writer.write(data)
        self.writer.write(b'\x00')
        await self.writer.drain()
        return await self._read_non_debug_message()

    async def set_light(self, *args, **kwargs):
        request = build_set_light(*args, **kwargs)
        await self.write(request)

    async def commit(self, *args, **kwargs):
        await self.write(build_commit(*args, **kwargs))
