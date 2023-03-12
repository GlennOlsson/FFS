import src.iozone_parser as iozone
import src.diagram as diagram

import src.sniff_parser as sniff_parser

import src.latex as latex

import os

from typing import Optional

BASE_INPUT_PATH = "../saved.nosync"
BASE_OUTPUT_PATH = "../../doc/figures.nosync/benchmarking"

class Benchmark:
	fs: str
	identifier: str
	with_cache: bool

	def __init__(self, fs: str, with_cache: bool):
		self.fs = fs
		self.with_cache = with_cache
		self.identifier = f"UBC {'Enabled' if self.with_cache else 'Disabled'}"

		self.sniff = None

	result: iozone.IOZoneResult
	sniff: Optional[sniff_parser.Sniff]

	def parse(self):
		path = f"{BASE_INPUT_PATH}/{self.fs}-{'with' if self.with_cache else 'no'}-cache"
		prefix = f".log"

		self.result = iozone.report(path, prefix, self.fs, self.identifier)

		sniff_path = f"{path}/Sniffs"
		if os.path.exists(sniff_path):
			self.sniff = sniff_parser.parse_dir(sniff_path)


	def generate(self):
		path = f"{BASE_OUTPUT_PATH}/{self.fs}"

		if not os.path.exists(path):
			os.mkdir(path)

		# diagram.draw_histograms(self.result, path)
		# diagram.draw_bfs_histograms(self.result, path)
		# latex.generate_tables(self.result, path)

		# diagram.draw_scatters(self.result, path)

		latex.generate_stat_tables(self.result, path)

		# if self.sniff is not None and not self.sniff.is_empty():
		# 	diagram.sniff_diagram(self.sniff, path)

	def time(self):
		print(f"Time for {self.fs} ({self.identifier})")
		iterations = 20
		total_time = 0
		for report in self.result:
			report_time = 0
			for file_size_entry in report:
				size = file_size_entry.size
				for dp in file_size_entry:
					val = dp.value # value = kb / s
					time_per_dp = size / val # == s
					report_time += time_per_dp
			# print(f"For {report.name}: {report_time}s")
			total_time += report_time
		
		print(f"Average time per benchmark: {total_time}s ({total_time/60}m == {(total_time/60)/60}h)")
		print(f"Total: {iterations * total_time}s ({iterations * total_time/60}m == {iterations * (total_time/60)/60}h)")
