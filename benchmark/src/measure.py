import benchmark.src.cloud_filesystem as cloud_filesystem
import benchmark.src.commands as commands
import benchmark.src.network_sniff as network_sniff

import os

from benchmark.src import logger

LOG_BASEPATH = os.getcwd() + "/logs"

def get_log_path(prefix: str, i: int):
	if not os.path.exists(LOG_BASEPATH):
		os.mkdir(LOG_BASEPATH)

	log_path = f"{LOG_BASEPATH}/{prefix}-iter-{i}"

	while os.path.exists(log_path): # Extend with new. Can be (new)(new)(new) worst case. But don't want to overwrite
		log_path += "(new)"
	
	log_path += ".log"

	return log_path

def get_sniff_path(prefix: str, i: int):
	if not os.path.exists(LOG_BASEPATH):
		os.mkdir(LOG_BASEPATH)

	log_path = f"{LOG_BASEPATH}/{prefix}-sniff-{i}"

	while os.path.exists(log_path): # Extend with new. Can be (new)(new)(new) worst case. But don't want to overwrite
		log_path += "(new)"
	
	log_path += ".log"

	return log_path

MAX_ATTEMPTS = 3
class TooManyAttempts(Exception):
	fs_name: str
	iteration: int

	def __init__(self, fs: str, iterations: int):
		self.fs_name = fs
		self.iteration = iterations

	def __str__(self):
		return f"Too many attempts on filesystem {self.fs_name} for iteration {self.iteration}. Attempted {MAX_ATTEMPTS} times"

class BenchmarkIteration:
	attempt: int
	iteration: int
	filesystem: cloud_filesystem.Filesystem
	sniffer: network_sniff.Sniffer

	def __init__(self, i: int, filesystem: cloud_filesystem.Filesystem):
		self.iteration = i
		self.filesystem = filesystem
		self.attempt = 0

		self.sniffer = network_sniff.Sniffer(
			output_file=get_sniff_path(
				self.filesystem.name, 
				self.iteration
			), 
			tmp_file=get_sniff_path(
				f"tmp-{self.filesystem.name}", 
				self.iteration
			)
		)
	
	def iozone_log_path(self):
		return get_log_path(f"{self.filesystem.name}-iozone", self.iteration)

	def log_path(self):
		return get_log_path(self.filesystem.name, self.iteration)
	
	def failed_log_path(self):
		return get_log_path(f"{self.filesystem.name}-FAILED-ATTEMPT-{self.attempt}", self.iteration)

	def benchmark_path(self):
		return f"{self.filesystem.path}/bench.tmp"
		
	def command(self):
		filesize_arg = "-s1024 -s2048 -s4096 -s8192 -s16384 -s32768 -s65536 -s131072"
		if self.filesystem.name != "GCSF":
			filesize_arg += " -s262144"

		iozone_command = f"iozone -R {filesize_arg} -c -i0 -i1 -i2 -f {self.benchmark_path()}"

		if self.filesystem.needs_sudo:
			iozone_command = "sudo " + iozone_command

		return iozone_command

	def _write_file(self, path: str, data: bytes):
		with open(path, "w+") as f:
			f.write(str(data, encoding="UTF-8"))

	def _write_successful(self, output: bytes):
		# If successful, write output (stdout from command) to iozone log and metadata to logfile
		self._write_file(self.iozone_log_path(), output)

	# Write the data we got SO far
	def _write_failed(self, output: bytes):
		self._write_file(self.failed_log_path(), output)

	def _execute_benchmark(self):
		logger.debug("Executing benchmark")
		self.sniffer.start()
		executed_command = commands.run(self.command())
		self.sniffer.stop()
		logger.debug("Benchmark has completed")

		output = executed_command.stdout
		try:
			executed_command.check_returncode()
		except commands.CalledProcessError as e:
			self._write_failed(output)
			logger.debug(f"CalledProcessError for attempt {self.attempt}: {str(e)}. Stderr: {e.stderr}, output: {e.output}")
			self.attempt += 1
			raise e
			
	def execute(self):
		# Assumes is mounted when calling execute
		while self.attempt < MAX_ATTEMPTS:
			try:
				# If successful, break loop
				self._execute_benchmark()
				return
			except commands.CalledProcessError:
				# If threw, just unmount, mount and run loop again
				logger.debug(f"Failed for attempt {self.attempt}")
				self.filesystem.unmount()
				self.filesystem.mount()
				pass
		
		# if loop is exhausted, too many attempts without returning
		raise TooManyAttempts(self.filesystem.name, self.iteration)

class BenchmarkState:
	iteration = 0
	filesystem: cloud_filesystem.Filesystem

	def __init__(self, fs: cloud_filesystem.Filesystem):
		self.filesystem = fs
	
	def set_iteration(self, iteration):
		self.iteration = iteration
	
	def run(self, total_iterations: int):
		self.filesystem.mount()

		end_i = self.iteration + total_iterations

		try:
			for i in range(self.iteration, end_i):
				logger.debug(f"Run iteration {i} for {self.filesystem.name}")
				iteration = BenchmarkIteration(i, self.filesystem)
				iteration.execute()
		except TooManyAttempts as e:
			logger.debug("!!!! Could not complete benchmarking !!!!")
			logger.debug(str(e))

			commands.send_email("FAILED", f"""Benchmarking has failed for {e.fs_name}

				Failed iteration: {e.iteration}

				Exiting program, please restart
			""")

			# Exit program
			exit(1)
		finally:
			self.filesystem.unmount()