from dataclasses import dataclass, asdict

@dataclass
class StackMeasurementPair:
    start: int
    end: int

    def decode(value):
        return StackMeasurementPair((value & 0xFFFF0000) >> 16, value & 0xFFFF)

def extract_pair(value):
    return StackMeasurementPair.decode(value)

@dataclass
class StackMeasurement:
    '''
    location according to source code
    '''
    id: int

    '''
    __data_start, __data_end
    '''
    data: StackMeasurementPair

    '''
    __bss_start, __bss_end
    '''
    bss: StackMeasurementPair

    '''
    __malloc_heap_start, __malloc_heap_end
    '''
    heap: StackMeasurementPair

    '''
    __brkval, __malloc_margin
    '''
    gap: StackMeasurementPair

    '''
    stack pointer, ram end
    '''
    stack: StackMeasurementPair

    def __init__(self, response):
        msg = response.stack_measurement
        self.id = msg.id
        self.data = extract_pair(msg.data)
        self.bss = extract_pair(msg.bss)
        self.heap = extract_pair(msg.heap)
        self.gap = extract_pair(msg.heap_gap)
        self.stack = extract_pair(msg.stack)

#    def __repr__(self):
#        return dedent(f'''
#        StackMeasurement(
#            id={self.id},
#            __data_start={self.data.start},
#            __data_end={self.data.end},
#            __bss_start={self.bss.start},
#            __bss_end={self.bss.end},
#            __malloc_heap_start={self.heap.start},
#            __malloc_heap_end={self.heap.end},
#            __brkval={self.gap.start},
#            __malloc_margin={self.gap.end},
#            SP={self.stack.start},
#            RAMEND={self.stack.end}
#        )
#        ''')

    def todict(self):
        return asdict(self)

