import benchmark.src.cloud_filesystem as cloud_filesystem

import os

class GCSF(cloud_filesystem.Filesystem):

	executable_path = "gcsf"
	
	@property
	def name(self) -> str:
		return "GCSF"

	@property
	def path(self):
		return f"{os.getcwd()}/gcsf"

	@property
	def needs_sudo(self) -> bool:
		return True

	def mount_cmd(self):
		return f"sudo {self.executable_path} mount gcsf --session ffs" 
	
	def unmount_cmd(self):
		return f"sudo umount {self.path}"
