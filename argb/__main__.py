print('ARGB Controller Started...')
from .coms import *
from .server import Server, Request
from .stack_measurement import StackMeasurement
from textwrap import dedent
import pandas as pd

def sequence(server):
    for x in range(0, 20):
        print(f'trying index: {x}')
        server.set_light(
                index=0,
                start=x,
                end=x+1,
                start_color=(255, 0, 0),
                end_color=(0, 0, 255),
                ahds=(50, 10, 50, 10))
        sleep(2)
        server.set_light(
                index=0,
                start=x,
                end=x+1,
                start_color=(0, 0, 0),
                end_color=(0, 0, 0),
                ahds=(0, 0, 0, 0))

class DebugMonitor:
    def __init__(self):
        self.stack_measurements = []

    def log(self, msg):
        print(f'debug monitor: {msg}')

    def ready(self, server):
        self.log('ready')
        self.current_time(server)

    def send_message(self, server):
        server.set_light(
            index=0,
            start=0,
            end=6,
            start_color=(255, 255, 255),
            end_color=(0, 0, 255),
            ahds=(5, 5, 5, 5))

        server.set_light(
            index=1,
            start=6,
            end=12,
            start_color=(255, 0, 0),
            start_color_alt=(0, 255, 0),
            end_color=(0, 0, 255),
            end_color_alt=(0, 255, 255),
            ahds=(5, 5, 5, 5))

        server.set_light(
            index=2,
            start=12,
            end=20,
            start_color=(255, 0, 0),
            start_color_alt=(255, 255, 0),
            end_color=(0, 255, 255),
            end_color_alt=(0, 0, 255),
            ahds=(5, 5, 5, 5))

    def current_time(self, server):
        request = Request()
        request.current_time_request = True
        server.connection.sendMessage(request)

    def process(self, server, msg):
        # Respond to messages here.
        if msg.HasField('stack_measurement'):
            #print('.')
            self.stack_measurements.append(StackMeasurement(msg))
        elif msg.HasField('log') and msg.log.id == 2:
            self.log('resending message')
            self.current_time(server)
        else:
            print(msg)
        return len(self.stack_measurements) >= 10

    def completed(self, server):
        self.log('completed')
        results = pd.json_normalize([obj.todict() for obj in self.stack_measurements])
        print(results.groupby('id').agg(['min', ('mode', lambda x: x.mode()), 'max']).transpose().to_string())
        for line in server.connection.received.hex('-').split('00'):
            print(line)

monitor = DebugMonitor()
server = Server('/dev/ttyACM0', monitor)
server.main()
