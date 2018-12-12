import os
import pipes
import shutil
import subprocess
import urllib.request

FILE_SYSTEM_DOWNLOAD_URL = 'http://tinycorelinux.net/9.x/x86_64/release/CorePure64-9.0.iso'
FILE_SYSTEM_FILE = 'root_file_system' + "." + FILE_SYSTEM_DOWNLOAD_URL.split(".")[-1]


class Container:

    def __init__(self, path):
        self.path = path if path.endswith("/") else path + "/"

    def setup(self):
        self._download_image()
        self._mount_file_system()

    def map(self, host_path, target_path):
        pass

    def run(self, executable):
        pass

    def _download_image(self):
        location = self.path + FILE_SYSTEM_FILE
        with urllib.request.urlopen(FILE_SYSTEM_DOWNLOAD_URL) as response, open(location, 'wb') as out_file:
            shutil.copyfileobj(response, out_file)

    def _mount_file_system(self):
        command = f"mount -o loop '{self.path + FILE_SYSTEM_FILE}' '{self.path}'"
        process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
        output, error = process.communicate()
        os.remove(self.path + FILE_SYSTEM_FILE)
