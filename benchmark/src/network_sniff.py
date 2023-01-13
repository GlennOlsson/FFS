import typing
import multiprocessing
from pathlib import Path

import benchmark.src.commands as commands

from benchmark.src import logger

# https://www.wireshark.org/docs/man-pages/tshark.html

# https://serverfault.com/a/973894

def start_sniffing(output_file: str, tmp_file: str):
	tshark_options = "-d tcp.port==80,http -q -z io,stat,10"
	cmd = f"tshark {tshark_options} -w {tmp_file}"

	with open(output_file, "w+") as f:
		logger.debug(f"Running sniff: {cmd}")
		commands.run(cmd, f)

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
		if self.sniffer_process is None:
			return
		if self.sniffer_process.is_alive():
			self.sniffer_process.kill()
		else:
			logger.debug("Cannot kill dead sniffer")