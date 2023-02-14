import src.iozone_parser as iozone
import src.diagram as diagram

import src.latex as latex

import src.sniff_parser as sniff_parser

import os

from typing import Optional

BASE_INPUT_PATH = "../saved.nosync"
BASE_OUTPUT_PATH = "../../doc/figures.nosync/benchmarking"

class Benchmark:
	fs: str
	with_cache: bool

	def __init__(self, fs: str, with_cache: bool):
		self.fs = fs
		self.with_cache = with_cache

		self.sniff = None

	result: iozone.IOZoneResult
	sniff: Optional[sniff_parser.Sniff]

	def parse(self):
		path = f"{BASE_INPUT_PATH}/{self.fs}-{'with' if self.with_cache else 'no'}-cache"
		prefix = f"iozone"
		identifier = f"UBC {'Enabled' if self.with_cache else 'Disabled'}"

		self.result = iozone.report(path, prefix, self.fs, identifier)

		sniff_path = f"{path}/Sniffs"
		if os.path.exists(sniff_path):
			self.sniff = sniff_parser.parse_dir(sniff_path)


	def generate(self):
		path = f"{BASE_OUTPUT_PATH}/{self.fs}"

		if not os.path.exists(path):
			os.mkdir(path)

		diagram.draw_histograms(self.result, path)
		latex.generate_tables(self.result, path)

		if self.sniff is not None:
			diagram.sniff_histogram(self.sniff, path)


def run():

	benchmarks = [
		Benchmark("FFS", True),
		Benchmark("FFS", False),
		Benchmark("GCSF", True),
		Benchmark("GCSF", False),
		Benchmark("APFS", True),
		Benchmark("APFS", False),
		Benchmark("FFFS", True),
		Benchmark("FFFS", False),
	]

	for bench in benchmarks:
		bench.parse()

		# bench.generate()
	
	diagram.draw_box_plots([bench.result for bench in benchmarks], BASE_OUTPUT_PATH)

	# sniff = sniff_parser.parse_dir("../saved.nosync/FFS-no-cache/Sniffs")
	# diagram.sniff_histogram(sniff, ".")

	# diagram.draw_histograms(results[0], ".")

	# latex.generate_tables(report, ".")

	# diagram.draw_box_plots(results, ".")

	# file = "../saved.nosync/FFS-no-cache/Sniffs/FFS-sniff-2.log"

	# s = sniff_parser.parse(file)

	# bps = []
	# for e in s.entries:
	# 	bps.append(e.bps())
	# bps.sort()

	# diagram.create_bell(bps)

	# print(bps)

	# print(s.avg_bps())