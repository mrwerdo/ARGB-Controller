from serial_asyncio import open_serial_connection

class Device:
    def __init__(self, device):
        self.port = device
        self.reader = None
        self.writer = None
        self.incoming = 0
        self.outgoing = 0
        self.crc = Crc(
            width=32,
            poly=0x04c11db7,
            reflect_in=True,
            xor_in=0xffffffff,
            xor_out=0xffffffff,
            reflect_out=True
        )

    async def __aenter__(self):
        self.reader, self.writer = await open_serial_connection(self.device, baudrate=9600)
        # Discard messages where the beginning has been missed.
        await self.reader.readuntil(separator=b'\x00')
        return self
    
    async def __aexit__(self, exc_type, exc, tb):
        print(exc_type, exc, tb)

    async def _read_packet(self):
        data = await self.reader.readutil(separator=b'\x00')
        if len(data) <= 5: 
            # data ends with the separator, and all messages
            # must contain a four byte crc.
            return
        
