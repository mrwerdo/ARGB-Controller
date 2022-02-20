print('ARGB Controller Started...')
from .coms import *
from .server import Server

server = Server('/dev/cu.usbmodem144201')


def light(**kwargs):
    print('updating light...')
    server.set_light(**kwargs)
    print(server.connection.receiveMessage())

with server.connection:
    sleep(1)
    print(server.connection.receiveMessage())
    light(index=0, start=0, end=20, start_color=(255, 255, 255), end_color=(0, 0, 255), ahds=(0, 0, 0, 50))
    # for x in range(0, 20):
        # print(f'trying index: {x}')
        # light(index=0, start=x, end=x+1, start_color=(255, 0, 0), end_color=(0, 0, 255), ahds=(50, 10, 50, 10))
        # sleep(2)
        # light(index=0, start=x, end=x+1, start_color=(0, 0, 0), end_color=(0, 0, 0), ahds=(0, 0, 0, 0))