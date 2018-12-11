class Container:

    def __init__(self, path: str):
        self.path = path

    def setup(self):
        self._download_image()
        self._extract_file_system()

    def map(self, host_path: str, target_path: str):
        pass

    def run(self, executable: str):
        pass

    def _download_image(self):
        pass

    def _extract_file_system(self):
        pass
