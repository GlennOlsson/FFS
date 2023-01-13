import benchmark.src.commands as commands
import benchmark.src.cloud_filesystem as filesystem
import benchmark.src.ffs as ffs
import benchmark.src.measure as measure
from benchmark.src import logger

def benchmark_filesystem(name: str, iterations: int, starting_at: int = 0):
	fs: filesystem.Filesystem
	if name.lower() == "ffs":
		fs = ffs.FFS()
	else:
		logger.debug(f"Filesystem {name} not covered")
		exit(1)
	
	logger.debug(f"Preparing for benchmarking {fs.name}")

	measurer = measure.BenchmarkState(fs)

	if starting_at > 0:
		measurer.set_iteration(starting_at)

	logger.debug(f"Running {iterations} iterations, starting at iteration {starting_at}")

	try:
		measurer.run(iterations)
	except Exception as e:
		logger.debug(f"Uncaught error: {str(e)}")
		commands.send_email("FAILED", f"""Uncaught error when running benchmark.
		
		{str(e)}
		""")
		return

	commands.send_email("SUCCEEDED", f"""Benchmarking for {name} was successfully completed.

	{iterations} iterations were run
	""")

def start():
	benchmark_filesystem("ffs", 1)

if __name__ == "__main__":
	start()