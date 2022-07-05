print('ARGB Controller Started...')
from .coms import *
from .server import Server
from typing import NamedTuple
from textwrap import dedent

class StackMeasurementPair(NamedTuple):
    start: int
    end: int

    def decode(value):
        return StackMeasurementPair((value & 0xFFFF0000) >> 16, value & 0xFFFF)


def extract_pair(value):
    return StackMeasurementPair.decode(value)


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


class StackMeasurement:
    def __init__(self, response):
        msg = response.stack_measurement
        self.id = msg.id
        self.data = extract_pair(msg.data)
        self.bss = extract_pair(msg.bss)
        self.heap = extract_pair(msg.heap)
        self.gap = extract_pair(msg.heap_gap)
        self.stack = extract_pair(msg.stack)

    def __repr__(self):
        return dedent(f'''
        StackMeasurement(
            id={self.id},
            __data_start={self.data.start},
            __data_end={self.data.end},
            __bss_start={self.bss.start},
            __bss_end={self.bss.end},
            __malloc_heap_start={self.heap.start},
            __malloc_heap_end={self.heap.end},
            __brkval={self.gap.start},
            __malloc_margin={self.gap.end},
            SP={self.stack.start},
            RAMEND={self.stack.end}
        )
        ''')


class DebugMonitor:
    def __init__(self):
        self.stack_measurements = []

    def log(self, msg):
        print(f'debug monitor: {msg}')

    def ready(self, server):
        self.log('ready')
        server.set_light(
            index=0,
            start=0,
            end=6,
            start_color=(255, 255, 255),
            end_color=(0, 0, 255),
            ahds=(5, 5, 5, 5)

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

    def process(self, server, msg):
        # Respond to messages here.
        if msg.HasField('stack_measurement'):
            print('.')
            self.stack_measurements.append(StackMeasurement(msg))
        else:
            print(msg)
        return len(self.stack_measurements) >= 100

    def completed(self, server):
        print('completed')
        return
        locations = {}
        for record in self.stack_measurements:
            value = locations.setdefault(record.id, []) 
            value.append(record)

        keys = sorted(locations.keys())
        for location in keys:
            print(f'Stack measured at location: {location}')
            for measurement in locations[location]:
                print(measurement)


monitor = DebugMonitor()
server = Server('/dev/ttyACM0', monitor)
server.main()
