import click
from .coms import *
from .server import Server
from .DebugMonitor import DebugMonitor

@click.option('-l', '--log', is_flag=True)
@click.command
def main(log):
    print('ARGB Controller Started...')
    monitor = DebugMonitor()
    server = Server('/dev/ttyACM0', monitor)
    server.connection.logging_enabled = log
    server.main()

if __name__=='__main__':
    main()

