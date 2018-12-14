import os
import re
import sys
import uuid
import time
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
        process = subprocess.Popen(['sudo', 'mkdir', path_in_container], stdout=subprocess.PIPE, cwd=self.path)
        process.communicate()

        process = subprocess.Popen(['sudo', 'mount', '--bind', host_path, path_in_container], stdout=subprocess.PIPE, cwd=self.path)
        process.communicate()

        process = subprocess.Popen(['sudo', 'mount', '-o', 'remount,ro', path_in_container], stdout=subprocess.PIPE, cwd=self.path)
        process.communicate()

    def run(self, join_namespace, limitations, executable, args):
        nsenter_command = self.create_nsenter_command(join_namespace)
        unshare_command = ['unshare', '--pid', '--fork', '--mount-proc=' + self.path + 'proc']
        chroot_command = ['chroot', self.path]
        cgroup_command = []

        if limitations:
            limited_resources, group_name = self.create_cgroup(limitations)
            cgroup_command = ['cgexec', '-g', limited_resources + ":" + group_name]

        process = subprocess.Popen(['sudo'] + nsenter_command + unshare_command + cgroup_command + chroot_command + [executable] + args, stdin=sys.stdin, stdout=sys.stdout, stderr=sys.stderr, cwd=self.path)
        process.communicate()

        if limitations:
            self.delete_cgroup(limited_resources, group_name)

    def create_nsenter_command(self, namespace_args):
        if not namespace_args or len(namespace_args) < 1:
            return []

        command = ['nsenter']

        for namespace_arg in namespace_args:
            split = namespace_arg.split('=')
            command += ['--' + split[0] + '=/proc/' + split[1] + '/ns/' + split[0]]

        return command

    def create_cgroup(self, limitations):
        group_name = uuid.uuid4().hex

        limited_resources = []
        limited_resource_keys=[]
        resource_limits = []

        for limitation in limitations:
            split = re.split('\.|=', limitation)
            limited_resources.append(split[0])
            limited_resource_keys.append(split[1])
            resource_limits.append(split[2])

        group_resources = ','.join(map(str, limited_resources))

        process = subprocess.Popen(['sudo', 'cgcreate', '-g', group_resources + ":" + group_name], stdout=subprocess.PIPE, cwd=self.path)
        process.communicate()

        for limitation in limitations:
            process = subprocess.Popen(['sudo', 'cgset', '-r', limitation, group_name], stdout=subprocess.PIPE, cwd=self.path)
            process.communicate()

        return (group_resources, group_name)

    def delete_cgroup(self, limited_resources, group_name):
        process = subprocess.Popen(['sudo', 'cgdelete', limited_resources + ":" + group_name], stdout=subprocess.PIPE, cwd=self.path)
        process.communicate()

    def _download_image(self):
        location = os.path.join(self.path, FILE_SYSTEM_FILE)
        urllib.request.urlretrieve(FILE_SYSTEM_DOWNLOAD_URL, location)

    def _mount_file_system(self):
        process = subprocess.Popen(['sudo', 'tar', 'xf', self.path + FILE_SYSTEM_FILE], stdout=subprocess.PIPE, cwd=self.path)
        process.communicate()
        os.remove(self.path + FILE_SYSTEM_FILE)
