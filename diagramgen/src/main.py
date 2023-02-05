import src.iozone_parser as iozone
import src.diagram as diagram

import src.sniff_parser as sniff_parser

from typing import List

def run():
	reports = iozone.parse_files("../saved.nosync/FFS-no-cache", "FFS-iozone")

	report = reports[0]
	for r in reports[1:]:
		report.extend(r)

	for r in report:
		r.print()

	# file = "../saved.nosync/FFS-no-cache/Sniffs/FFS-sniff-2.log"

	# s = sniff_parser.parse(file)

	# bps = []
	# for e in s.entries:
	# 	bps.append(e.bps())
	# bps.sort()

	# diagram.create_bell(bps)

	# print(bps)

	# print(s.avg_bps())