import src.iozone_parser as iozone
import src.diagram as diagram

import src.latex as latex

import src.sniff_parser as sniff_parser

import src.stats as stats

import src.models as models

import os

from typing import Optional

def run():

	benchmarks = [
		models.Benchmark("FFS", True), # 0 
		models.Benchmark("FFS", False), # 1
		models.Benchmark("GCSF", True), # 2
		models.Benchmark("GCSF", False), # 3
		models.Benchmark("FFFS", True), # 4
		models.Benchmark("FFFS", False), # 5
		models.Benchmark("APFS", True), # 6
		models.Benchmark("APFS", False), # 7
	]

	for bench in benchmarks:
		bench.parse()

		# bench.time()
		bench.generate()

		# # for report in bench.result:
		# # 	print(f"{bench.fs} {bench.identifier} - {report.name}")
		# # 	stats.joint_distribution(report)
		# break
	# ffs = benchmarks[1]

	# stats.confidence_interval(ffs.result["Write"])
	# while True:
	# 	try:
	# 		latex.generate_bootstrap_tables(benchmarks, models.BASE_OUTPUT_PATH)
	# 		break
	# 	except:
	# 		pass

	# latex.generate_stat_tables(benchmarks, models.BASE_OUTPUT_PATH)

	# for report in ffs.result:
	# 	print(report.name)
	# 	for size_report in report:
	# 		print("\t", size_report.size)
	# 		stats.cohensd(size_report.raw_values())

	# diagram.sniff_diagram(ffs.sniff, ".")

	# fffs = benchmarks[4]

	# print("FFS", sorted(ffs.result["Write"].raw_values(), reverse=True)[:10])
	# print("FFFS", sorted(fffs.result["Write"].raw_values(), reverse=True)[:10])
	
	# diagram.draw_box_plots([bench.result for bench in benchmarks], BASE_OUTPUT_PATH)

	# stats.cohensd(benchmarks[5].result["Write"][16384].raw_values())

	# latex.generate_sniff_table(
	# 	[(
	# 		f"{benchmark.fs}, {benchmark.identifier}",
	# 		 benchmark.sniff
	# 	) for benchmark in benchmarks if benchmark.sniff is not None and not benchmark.sniff.is_empty()]
	# 	, BASE_OUTPUT_PATH
	# )

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