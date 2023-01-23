import benchmark.src.commands as commands

import abc
import time
import multiprocessing
import typing

from benchmark.src import logger

def _mount_in_process(command):
	logger.debug(f"Mounting with command {command}")
	commands.run(command)

class Filesystem(abc.ABC):

	mount_process: typing.Optional[multiprocessing.Process]

	@abc.abstractproperty
	@property
	def name(self) -> str:
		pass

	@abc.abstractproperty
	@property
	def path(self) -> str:
		pass

	@abc.abstractproperty
	@property
	def needs_sudo(self) -> bool:
		pass

	@abc.abstractmethod
	def mount_cmd(self) -> str:
		pass
	
	@abc.abstractmethod
	def unmount_cmd(self) -> str:
		pass

	def mount(self):
		logger.debug("Mounting")
		cmd = self.mount_cmd()

		# Using process rather than thread because the GID of python can only execute one non-blocking thread at the same time
		process = multiprocessing.Process(target=_mount_in_process, args=[cmd])
		process.start()

		# Join for a few seconds to let it mount. When timeout is done, it's no longer blocking and the
		# original process will return
		process.join(timeout=5)

		self.mount_process = process
		logger.debug("Mounted")
	
	def unmount(self):
		logger.debug("Unmounting")
		cmd = self.unmount_cmd()

		if self.mount_process.is_alive():
			self.mount_process.kill()
		else:
			logger.debug("Cannot kill dead mount_process")

		commands.run(cmd)

		time.sleep(5)
		
		logger.debug("Unmounted")