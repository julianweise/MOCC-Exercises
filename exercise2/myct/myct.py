import argparse
import os

from container import Container
from parser_accessible_dir import AccessibleDir


def main():
    parser = _initialize_parser()

    args = parser.parse_args()

    if 'action' not in args or not args.action:
        parser.print_usage()
        return

    container = Container(args.container_path)
    if args.action == 'init':
        container.setup()
    elif args.action == 'map':
        container.map(args.host_path, args.target_path)
    elif args.action == 'run':
        # TODO: Handle limit and namespace / Probably set directly on container
        container.run(args.limit, args.executable, args.executable_args)


def _initialize_parser():
    general_parser = argparse.ArgumentParser(description='Small container tool')

    subparsers = general_parser.add_subparsers()

    _initialize_init_parser(subparsers)
    _initialize_map_parsers(subparsers)
    _initialize_run_parser(subparsers)

    return general_parser


def _initialize_init_parser(subparsers):
    init_parser = subparsers.add_parser('init', help='Initialize new Container')
    init_parser.set_defaults(action='init')
    init_parser.add_argument('container_path', help='Where to initialize the container to?', action=AccessibleDir)


def _initialize_map_parsers(subparsers):
    map_parser = subparsers.add_parser('map', help='Mount a host directory read-only into the container at destination')
    map_parser.set_defaults(action='map')
    map_parser.add_argument('container_path', help='Which container to map to?', action=AccessibleDir)
    map_parser.add_argument('host_path', help='Which directory to map?', action=AccessibleDir)
    map_parser.add_argument('target_path', help='Where to map the directory to?')


def _initialize_run_parser(subparsers):
    run_parser = subparsers.add_parser('run', help='Runs the file exectuable in container with passed arguments')
    run_parser.set_defaults(action='run')
    run_parser.add_argument('container_path', help='Which container to run?', action=AccessibleDir)
    run_parser.add_argument('--namespace', type=str, help='Define a namespace to join: Format: <kind>=<pid>')
    run_parser.add_argument('--limit', type=str, help='Limit resources; Format: <controller.key>=<value>', action='append')
    run_parser.add_argument('executable', help='Which executable to run?')
    run_parser.add_argument('executable_args', type=str, nargs='*', help='Arguments for the executable.')


def _is_valid_directory(path):
    return os.path.isdir(path)


if __name__ == "__main__":
    main()
