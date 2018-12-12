import os
import shutil
import subprocess
import urllib.request

FILE_SYSTEM_DOWNLOAD_URL = 'https://cloud-images.ubuntu.com/minimal/releases/bionic/release/' \
                           'ubuntu-18.04-minimal-cloudimg-amd64-root.tar.xz'
FILE_SYSTEM_FILE = 'root_file_system' + ".tar.xz"


class Container:

    def __init__(self, path):
        self.path = path if path.endswith("/") else path + "/"

    def setup(self):
        self._download_image()
        self._mount_file_system()

    def map(self, host_path, target_path):
        path_in_container = self.path + target_path[1:] if target_path[0] == "/" else self.path + target_path
        process = subprocess.Popen(['ln', '-s', host_path, path_in_container], stdout=subprocess.PIPE, cwd=self.path)
        process = subprocess.Popen(['chmod', '555', path_in_container], stdout=subprocess.PIPE, cwd=self.path)
        # output, error = process.communicate()

    def run(self, executable):
        pass

    def _download_image(self):
        location = os.path.join(self.path, FILE_SYSTEM_FILE)
        with urllib.request.urlopen(FILE_SYSTEM_DOWNLOAD_URL) as response, open(location, 'wb') as out_file:
            shutil.copyfileobj(response, out_file)

    def _mount_file_system(self):
        process = subprocess.Popen(['tar', 'xf', self.path + FILE_SYSTEM_FILE], stdout=subprocess.PIPE, cwd=self.path)
        # output, error = process.communicate()
        os.remove(self.path + FILE_SYSTEM_FILE)
