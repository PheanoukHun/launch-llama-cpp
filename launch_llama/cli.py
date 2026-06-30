import argparse
import sys

def create_parser():
    parser = argparse.ArgumentParser(
        prog='launch-llama',
        description='Launch and manage llama server instances'
    )
    parser.add_argument(
        '--verbose', '-v', action='store_true',
        help='Enable verbose/debug output'
    )
    parser.add_argument(
        '--no-confirm', action='store_true',
        help='Skip confirmation prompt before running command'
    )
    parser.add_argument(
        '--version', action='store_true',
        help='Show version and exit'
    )

    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    run_parser = subparsers.add_parser('run', help='Run a model or preset')
    run_parser.add_argument('preset', nargs='?', help='Preset name or "llama-server" or "llama-swap"')
    run_parser.add_argument('-p', '--port', type=int, help='Port number (overrides config)')

    return parser

def parse_args(args=None):
    parser = create_parser()
    parsed = parser.parse_args(args)
    if not parsed.command:
        parsed.command = None
    return parsed
