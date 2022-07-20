import click
from .coms import *
from .server import Server
from .DebugMonitor import DebugMonitor
from .AsyncServer import AsyncServer

@click.option('-l', '--log', is_flag=True)
@click.option('-a', '--async-runtime', is_flag=True)
@click.command
def main(log, async_runtime):
    print('ARGB Controller Started...')
    if async_runtime:
        monitor = DebugMonitor()
        server = AsyncServer('/dev/ttyACM0', monitor)
        server.logging_enabled = log
        server.main()
    else:
        monitor = DebugMonitor()
        server = Server('/dev/ttyACM0', monitor)
        server.connection.logging_enabled = log
        server.main()

if __name__=='__main__':
    main()

