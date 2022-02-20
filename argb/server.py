from .coms import Connection
from messages_pb2 import Request

def pack_rgb(v):
    a, b, c = v
    return (a << 16) | (b << 8) | c

def pack_ahds(v):
    a, h, d, s = v
    return (a << 24) | (h << 16) | (d << 8) | s


class Server:
    def __init__(self, device):
        self.connection = Connection(device)

    def set_light(self, index, start, end, start_color, end_color, ahds, start_color_alt=None, end_color_alt=None):
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
        self.connection.sendMessage(request)