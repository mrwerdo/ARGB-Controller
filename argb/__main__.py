print('ARGB Controller Started...')
from .coms import *
from .server import Server

server = Server('/dev/ttyACM0')


def light(**kwargs):
    print('updating light...')
    server.set_light(**kwargs)
    print(server.connection.receiveMessage())

with server.connection:
    sleep(1)
    if server.connection.serialPort.in_waiting != 0:
        print(server.connection.receiveMessage())
    light(index=0, start=0, end=6, start_color=(255, 255, 255), end_color=(0, 0, 255), ahds=(5, 5, 5, 5))
    light(index=1, start=6, end=12, start_color=(255, 0, 0), start_color_alt=(0, 255, 0), end_color=(0, 0, 255), end_color_alt=(0, 255, 255), ahds=(5, 5, 5, 5))
    light(index=2, start=12, end=20, start_color=(255, 0, 0), start_color_alt=(255, 255, 0), end_color=(0, 255, 255), end_color_alt=(0, 0, 255), ahds=(5, 5, 5, 5))
#    for x in range(0, 20):
#        print(f'trying index: {x}')
#        light(index=0, start=x, end=x+1, start_color=(255, 0, 0), end_color=(0, 0, 255), ahds=(50, 10, 50, 10))
#        sleep(2)
#        light(index=0, start=x, end=x+1, start_color=(0, 0, 0), end_color=(0, 0, 0), ahds=(0, 0, 0, 0))
