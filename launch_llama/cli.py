import argparse
import sys

def create_parser():
    parser = argparse.ArgumentParser(
        prog='launch-llama',
        description='Launch and manage llama server instances'
    )
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    run_parser = subparsers.add_parser('run', help='Run a model or preset')
    run_parser.add_argument('preset', nargs='?', help='Preset name or "llama-server"')
    run_parser.add_argument('-p', '--port', type=int, help='Port number')
    
    return parser

def parse_args(args=None):
    parser = create_parser()
    parsed = parser.parse_args(args)
    if not parsed.command:
        parsed.command = None
    return parsed

if __name__ == '__main__':
    parsed = parse_args()
    print(parsed)
