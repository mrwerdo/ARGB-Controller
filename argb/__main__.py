print('ARGB Controller Started...')
from .coms import *
from .server import Server
from typing import NamedTuple
from textwrap import dedent

server = Server('/dev/ttyACM0')

def light(**kwargs):
    print('updating light...')
    server.set_light(**kwargs)

class StackMeasurementPair(NamedTuple):
    start: int
    end: int

def extract_pair(value) -> StackMeasurementPair:
    return StackMeasurementPair((value & 0xFFFF0000) >> 16, value & 0xFFFF)

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

with server.connection:
    sleep(1)
    if server.connection.serialPort.in_waiting != 0:
        print(server.connection.receiveMessage())
    light(index=0, start=0, end=6, start_color=(255, 255, 255), end_color=(0, 0, 255), ahds=(5, 5, 5, 5))
    #light(index=1, start=6, end=12, start_color=(255, 0, 0), start_color_alt=(0, 255, 0), end_color=(0, 0, 255), end_color_alt=(0, 255, 255), ahds=(5, 5, 5, 5))
    #light(index=2, start=12, end=20, start_color=(255, 0, 0), start_color_alt=(255, 255, 0), end_color=(0, 255, 255), end_color_alt=(0, 0, 255), ahds=(5, 5, 5, 5))
    while True:
        if server.connection.serialPort.in_waiting != 0:
            msg = server.connection.receiveMessage()
            if msg is not None and msg.HasField('stack_measurement'):
                stack_measurement = StackMeasurement(msg)
                print(stack_measurement)
            else:
                print(msg)
#    for x in range(0, 20):
#        print(f'trying index: {x}')
#        light(index=0, start=x, end=x+1, start_color=(255, 0, 0), end_color=(0, 0, 255), ahds=(50, 10, 50, 10))
#        sleep(2)
#        light(index=0, start=x, end=x+1, start_color=(0, 0, 0), end_color=(0, 0, 0), ahds=(0, 0, 0, 0))
