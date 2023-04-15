import typing
import multiprocessing
from pathlib import Path
import os
import signal
import subprocess

import benchmark.src.commands as commands

from benchmark.src import logger

# https://www.wireshark.org/docs/man-pages/tshark.html

# https://serverfault.com/a/973894

interface = "en9"

def start_sniffing(output_file: str, tmp_file: str):
	tshark_options = f"-i {interface} -d tcp.port==80,http -q -z io,stat,10 -b filesize:1000000"
	cmd = f"tshark {tshark_options} -w {tmp_file}"

	logger.debug(f"Running sniff: {cmd}")
	file = open(output_file, "a+")
	proc: subprocess.Popen[bytes] = commands.popen(cmd, file)
	try:
		proc.wait()
	except KeyboardInterrupt:
		logger.debug("Got keyboard interrupt (SIGINT)")
		proc.send_signal(signal.SIGINT)
		proc.wait()
		file.close()
		logger.debug(f"Killed tshark command")

class Sniffer:

	sniffer_process: typing.Optional[multiprocessing.Process]
	
	result_file: str
	tmp_file: str

	def __init__(self, output_file: str, tmp_file: str):
		self.tmp_file = tmp_file
		self.result_file = output_file
		self.sniffer_process = None

	def start(self):
		process = multiprocessing.Process(target=start_sniffing, args=[self.result_file, self.tmp_file])
		process.start()
		# Join for a few seconds to let it start recording
		process.join(timeout=5)

		self.sniffer_process = process

	def stop(self):
		if self.sniffer_process is None or not self.sniffer_process.is_alive():
			logger.debug("Cannot kill dead sniffer")
		else:
			os.kill(self.sniffer_process.pid, signal.SIGINT)
			# self.sniffer_process.kill()
			logger.debug("Killed sniffer")
		
		if os.path.exists(self.tmp_file):
			os.remove(self.tmp_file)
			logger.debug("Removed tmp sniff file")
		logger.debug("Stopped sniffer")