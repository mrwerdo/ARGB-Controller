from psutil import sensors_temperatures
from argb.Device import Device, DebugDelegate
from math import cos, pi

_cpu_max = None
_cpu_min = None
def get_cpu_temperature(readings):
    global _cpu_max, _cpu_min
    result = readings['k10temp'][0].current
    if _cpu_max is None or _cpu_max < result:
        _cpu_max = result
    if _cpu_min is None or result < _cpu_min:
        _cpu_min = result
    return (result, _cpu_min, _cpu_max)

def color(angle):
    # float offset = z.length_squared();
    # float hue = (i+1-log2(log10(offset)/2))/maxiters*4 * M_PI_F + 3;
    # return float4((cos(hue)+1)/2,
    #               (-cos(hue+M_PI_F/3)+1)/2,
    #               (-cos(hue-M_PI_F/3)+1)/2,
    #               1);
    r = (cos(angle) + 1)/2
    g = (-cos(angle + pi / 3) + 1)/2
    b = (-cos(angle - pi / 3) + 1)/2
    return (min(int(255 * r), 255), min(int(255 * g), 255), min(int(255 * b), 255))


async def update_lights(device, current, minimum, maximum):
    if maximum == minimum:
        maximum = minimum + 1
    hue = 2 * int(pi) * float(current - minimum) / float(maximum - minimum)
    print(f'hue: {hue}')
    await device.set_light(
            index=2, 
            start=12, 
            end=19,
            start_color=color(hue),
            end_color=color(hue),
            ahds=(0, 5, 0, 0))

async def main():
    async with Device('/dev/ttyACM0', DebugDelegate()) as device:
        while True:
            readings = sensors_temperatures()
            current, minimum, maximum = get_cpu_temperature(readings)
            print(f'cpu temperature: {current} C out of {maximum} C')
            await update_lights(device, current, minimum, maximum)
            await device.commit(0)

import asyncio
asyncio.run(main())
