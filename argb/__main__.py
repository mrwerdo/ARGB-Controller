print('ARGB Controller Started...')
from .coms import *
from .server import Server

server = Server('/dev/cu.usbmodem14201')


def light(**kwargs):
    print('updating light...')
    server.set_light(**kwargs)
    print(server.connection.receiveMessage())

with server.connection:
    sleep(1)
    print(server.connection.receiveMessage())
    for x in range(0, 20):
        print(f'trying index: {x}')
        light(index=0, start=x, end=x+1, start_color=(255, 0, 0), end_color=(255, 0, 0), ahds=(10, 10, 10, 10))
        sleep(1)
        light(index=0, start=x, end=x+1, start_color=(0, 0, 0), end_color=(0, 0, 0), ahds=(10, 10, 10, 10))