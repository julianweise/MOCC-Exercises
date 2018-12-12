import argparse
import os
import sys


class AccessibleDir(argparse.Action):

    def __call__(self, parser, parser_namespace, values, option_string=None):
        directory = values

        if not os.path.isdir(directory):
            raise argparse.ArgumentError(self, "{0} is not a valid path".format(directory))

        if os.access(directory, os.R_OK):
            if not directory.endswith("/"):
                directory = directory + "/"
            if not directory.startswith("/") or not directory.startswith("~"):
                directory = os.path.abspath(os.path.dirname(sys.argv[0])) + "/" + directory
            setattr(parser_namespace, self.dest, directory)
        else:
            raise argparse.ArgumentError(self, "Permission denied to read from {0}".format(directory))
