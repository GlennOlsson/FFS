import src.iozone_parser as iozone
import src.diagram as diagram

import src.latex as latex

import src.sniff_parser as sniff_parser

from typing import List

def run():
	report1 = iozone.report("../saved.nosync/FFS-no-cache", "FFS-iozone")
	report1.identifier = "UBC disabled"

	report2 = iozone.report("../saved.nosync/FFS-with-cache", "FFS-iozone")
	report2.identifier = "UBC enabled"


	# diagram.draw_histograms(report, ".")

	# latex.generate_tables(report, ".")

	diagram.draw_box_plots([report1, report2], ".")

	# file = "../saved.nosync/FFS-no-cache/Sniffs/FFS-sniff-2.log"

	# s = sniff_parser.parse(file)

	# bps = []
	# for e in s.entries:
	# 	bps.append(e.bps())
	# bps.sort()

	# diagram.create_bell(bps)

	# print(bps)

	# print(s.avg_bps())