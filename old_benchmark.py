import functools
from importlib.metadata import files
import os
import subprocess
import random
import threading
import time
from typing import Callable, List

TEST_ITERATIONS = 1

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

mount_sleep = 5

# test name points to list of runs with their time
test_log: dict[str, list[int]] = {}

def _path(path):
	return f"{filesystem}/{path}"

# Writes verifiable bytes to the file
def generate_file(size: int, path: str):
	# Generate same content for all files with same size
	random.seed(size)
	file_content = random.randbytes(size)
	with open(_path(path), "wb") as f:
		f.write(file_content)

# Reads whole file and verifies content is as expected
def verify_file(path):
	with open(_path(path), "rb") as f:
		file_content = f.read()
		# Same seed as it was supposedly generated with
		file_len = len(file_content)
		random.seed(file_len)
		expected_file_content = random.randbytes(file_len)

		index = 0
		for (a,b) in zip(file_content, expected_file_content):
			if a != b:
				raise RuntimeError(f"FILE NOT OK at {index}")
			index += 1
		return True

def mount():
	def _mount():
		cmd = cmd_mount[filesystem]
		global mount_process
		mount_process = subprocess.Popen(cmd.split(" "))

	t = threading.Thread(target=_mount)
	t.start()
	time.sleep(mount_sleep) # Allow to mount and init itself

# Killing process, resetting memory cache
def unmount():
	cmd = f"umount {filesystem}/"
	subprocess.run(cmd.split(" "))
	mount_process.kill()
	mount_process.terminate()
	subprocess.run(cmd.split(" "))

def log_test(test_name: str, time: int):
	if test_name not in test_log:
		test_log[test_name] = []
		
	test_log[test_name].append(time)

tests: List[Callable[[], None]] = []

def bench_test(func: Callable[[], None]):
	def wrapper():
		# mount()
		pre_ns = time.time_ns()
		
		func()

		post_ns = time.time_ns()
		# unmount()

		total_time = (post_ns - pre_ns)

		log_test(func.__name__, total_time)
	
	tests.append(wrapper)

	return wrapper

## TESTS
## Will be run in order from defined

# @bench_test
# def save_100b_in_root():
# 	generate_file(100, "foo.txt")

# @bench_test
# def read_root_file():
# 	verify_file("foo.txt")

# @bench_test
# def remove_root_file():
# 	os.remove(_path("foo.txt"))

# @bench_test
# def create_level1_dir():
# 	os.mkdir(_path("bar"))

# @bench_test
# def create_level2_dir():
# 	os.mkdir(_path("bar/fizz"))

# @bench_test
# def save_100kb_in_level2():
# 	generate_file(100_000, "bar/fizz/buzz.txt")

# @bench_test
# def read_100kb_in_level2():
# 	verify_file("bar/fizz/buzz.txt")

# @bench_test
# def move_100kb():
# 	os.rename(_path("bar/fizz/buzz.txt"), _path("bar/jizz.txt"))

# @bench_test
# def remove_100kb():
# 	os.remove(_path("bar/jizz.txt"))

# @bench_test
# def remove_level2():
# 	os.rmdir(_path("bar/fizz"))

# @bench_test
# def create_empty_file_level1():
# 	open(_path("bar/empty.txt"), 'a').close()

# @bench_test
# def remove_empty_file_level1():
# 	os.remove(_path("bar/empty.txt"))

# @bench_test
# def remove_level1():
# 	os.rmdir(_path("bar"))

# @bench_test
# def create_empty_file_root():
# 	open(_path("empty.txt"), 'a').close()	

# @bench_test
# def remove_empty_file():
# 	os.remove(_path("empty.txt"))

@bench_test
def create_7mb_file():
	generate_file(200_000_000, "bigboy.txt")

@bench_test
def read_7mb_file():
	verify_file("bigboy.txt")

@bench_test
def remove_7mb_file():
	os.remove(_path("bigboy.txt"))

# Run tests, one at a time. After all has been run, run again. TEST_ITERATIONS times in total
def run_tests():
	for i in range(TEST_ITERATIONS):
		for f in tests:
			print("Run ", f.__name__)
			f()
		print(f"Done with iter {i}")

if __name__ == "__main__":
	while filesystem not in filesystems:
		print("Which filesystem: ")
		filesystem = input()

	print(f"Running {len(tests)} tests {TEST_ITERATIONS} times, at least {len(tests) * mount_sleep * TEST_ITERATIONS}s")

	start = time.time_ns()
	run_tests()
	end = time.time_ns()

	total_time = (end - start) / 1_000_000_000

	with open(f"{filesystem}_bench.log", "w") as log:
		log.write("---- TESTS COMPLETE ----\n")
		log.write(f"Ran {len(tests)} tests {TEST_ITERATIONS} times for {filesystem}\n")
		log.write(f"Total of {total_time}s\n")
		for test_name in test_log:
			results = test_log[test_name]
			avg_ns = sum(results) / TEST_ITERATIONS
			res_str = functools.reduce(lambda pre, curr: pre + str(curr) + "\n", results, "")
			log.write(f"{test_name}: {avg_ns}ns\t({avg_ns / 1_000_000_000}s)")
			log.write(f"\n{res_str}\n")

# Writes slower for FFS than GCSF
# Makes a lot of sense as it has to handle the inode table (upload it), requiring at 
# 	least one more request than GCSF as that is managed by Drive
# Read is faster for FFS than GCSF - better performance! 1.02s vs 0.79s for 10x 100b test
