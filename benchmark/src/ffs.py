import benchmark.src.cloud_filesystem as cloud_filesystem

import os

class FFS(cloud_filesystem.Filesystem):

	executable_path = "./out.nosync/main.out"
	
	@property
	def name(self) -> str:
		return "FFS"

	@property
	def path(self):
		return f"{os.getcwd()}/ffs"

	@property
	def needs_sudo(self) -> bool:
		return False

	def mount_cmd(self):
		return f"{self.executable_path} fuse {self.path} -f -s -o iosize=100000000000 -o nolocalcaches"
	
	def unmount_cmd(self):
		return f"umount {self.path}"
