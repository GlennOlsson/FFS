import src.iozone_parser as iozone
import src.diagram as diagram

import src.latex as latex

import src.sniff_parser as sniff_parser

from typing import List, Tuple

def parse_dirs(dirs: List[Tuple[str, str, str, str]]):
	return [iozone.report(path, prefix, fs, id) for path, prefix, fs, id in dirs]

def run():

	results = parse_dirs([
		("../saved.nosync/FFS-no-cache", "FFS-iozone", "FFS", "UBC Disabled"),
		("../saved.nosync/FFS-with-cache", "FFS-iozone", "FFS", "UBC Enabled"),
		("../saved.nosync/GCSF-no-cache", "GCSF-iozone", "GCSF", "UBC Disabled"),
		("../saved.nosync/GCSF-with-cache", "GCSF-iozone", "GCSF", "UBC Enabled"),
	])

	# diagram.draw_histograms(report, ".")

	# latex.generate_tables(report, ".")

	diagram.draw_box_plots(results, ".")

	# file = "../saved.nosync/FFS-no-cache/Sniffs/FFS-sniff-2.log"

	# s = sniff_parser.parse(file)

	# bps = []
	# for e in s.entries:
	# 	bps.append(e.bps())
	# bps.sort()

	# diagram.create_bell(bps)

	# print(bps)

	# print(s.avg_bps())