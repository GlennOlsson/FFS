import subprocess
import random
import threading
import time
from typing import Callable, List

TEST_ITERATIONS = 10

filesystem: str = " "
mount_process: subprocess.Popen

filesystems = [
	"ffs",
	"gcsf"
]

cmd_mount = {
	"ffs": "./out.nosync/main.out fuse ffs -f -s",
	"gcsf": "gcsf mount gcsf --session ffs"
}

def _path(path):
	return f"{filesystem}/{path}"

def generate_file(size: int, path: str):
	# Generate same content for all files with same size
	random.seed(size)
	file_content = random.randbytes(size)
	with open(_path(path), "wb") as f:
		f.write(file_content)

def verify_file(path):
	with open(_path(path), "rb") as f:
		file_content = f.read()
		# Same seed as it was supposedly generated with
		file_len = len(file_content)
		random.seed(file_len)
		expected_file_content = random.randbytes(file_len)

		for (a,b) in zip(file_content, expected_file_content):
			if a != b:
				raise "FILE NOT OK"
		return True

def mount():
	def _mount():
		cmd = cmd_mount[filesystem]
		global mount_process
		mount_process = subprocess.Popen(cmd.split(" "))

	t = threading.Thread(target=_mount)
	t.start()
	time.sleep(1) # Allow to mount and init itself

# Killing process, resetting memory cache
def unmount():
	cmd = f"umount {filesystem}/"
	subprocess.run(cmd.split(" "))
	mount_process.kill()
	mount_process.terminate()
	subprocess.run(cmd.split(" "))

result_log: List[str] = []
def log_test(test_name: str, time: int):
	avg_time = time / TEST_ITERATIONS
	log_entry = f"{test_name} took {avg_time}ns ({avg_time/1000000000}s)"
	result_log.append(log_entry)
	print(log_entry)

tests: List[Callable[[], None]] = []

def bench_test(func: Callable[[], None]):
	def wrapper():

		total_time = 0
		for _ in range(TEST_ITERATIONS):
			mount()
			pre_ns = time.time_ns()
			func()
			post_ns = time.time_ns()
			unmount()

			total_time += (post_ns - pre_ns)

		log_test(func.__name__, total_time)
	
	tests.append(wrapper)

	return wrapper

## TESTS
## Will be run in order from defined

@bench_test
def save_100b_in_root():
	generate_file(100, "foo.txt")

@bench_test
def verify_root_file():
	verify_file("foo.txt")

@bench_test
def verify_root_file_again():
	# Make sure reading file again is not cached
	verify_file("foo.txt")

def run_tests():
	result_log.append(f"Running {len(tests)} tests {TEST_ITERATIONS} times")
	for f in tests:
		print("Run ", f.__name__)
		f()

if __name__ == "__main__":
	while filesystem not in filesystems:
		print("Which filesystem: ")
		filesystem = input()
	
	run_tests()

	print("---- TESTS COMPLETE ----")
	for l in result_log:
		print(l)
