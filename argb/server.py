from time import sleep
from traceback import format_exc

from .coms import Connection
from .messages import Request, DebugMessage
from .messages import set_light as build_set_light

class Server:
    def __init__(self, device, delegate):
        self.connection = Connection(device)
        self.delegate = delegate

    def set_light(self, *args, **kwargs):
        request = build_set_light(*args, **kwargs)
        self.connection.sendMessage(request)

    def commit(self, delta):
        request = Request()
        message = request.commit_transaction
        message.timestamp = delta
        self.connection.sendMessage(request)

    def send_request(self, msg):
        self.connection.sendMessage(msg)

    def receiveMessage(self):
        if self.connection.serialPort.in_waiting != 0:
            return self.connection.receiveMessage()
        else:
            return None

    def main(self):
        with self.connection:
            self.connection.ignore_until_next_message()
            while True:
                try:
                    msg = self.receiveMessage()
                    if msg is None:
                        sleep(0.3)
                        continue
                    if msg.HasField('log') and msg.log.id == 9:
                        self.delegate.ready(self)
                        continue
                    should_stop = self.delegate.process(self, msg)
                    if should_stop:
                        break
                except KeyboardInterrupt:
                    print(format_exc())
                    break
                except ValueError:
                    print(f'server debug: {msg}')
            self.delegate.completed(self)
