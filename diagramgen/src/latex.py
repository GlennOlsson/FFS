from src.iozone_models import IOZoneResult, IOZoneReport
from src.sniff_models import Sniff
from src.stats import average, median, joint_distribution, confidence_interval
from src.models import Benchmark

from typing import Dict, List, Tuple

from functools import reduce

def underscore_case(s: str):
	return s.replace(" ", "_").replace("(", "").replace(")", "").lower()

def generate_table(report: IOZoneReport, fs: str, identifier: str) -> str:
	table = f"""\\begin{{table}}[!ht]
	\\begin{{center}}
	\\caption{{Average IOZone result for the {report.name} ({identifier}) test on {fs.upper()} in kilobytes per second}}"""
	
	# table_data[file_size][rec_len] is a list of values. These will later be averaged
	table_data: Dict[int, Dict[int, List[int]]] = {}

	for file_size_report in report:
		file_size = file_size_report.size
		if file_size not in table_data:
			table_data[file_size] = {}
		
		for dp in file_size_report:
			buffer_size = dp.buffer_size
			if buffer_size not in table_data[file_size]:
				table_data[file_size][buffer_size] = []
			
			table_data[file_size][buffer_size].append(dp.value)

	# List of buffer sizes is the maximum file size's total buffer sizes
	biggest_file_size = max(table_data.keys())
	buffer_sizes = table_data[biggest_file_size].keys()

	table += """\\resizebox{\\textwidth}{!}{
		\\begin{tabular}{| r """ + "| r " * len(buffer_sizes) + """| }
			\\hline
			{} & \multicolumn{"""+ str(len(buffer_sizes)) +"""}{c |}{Buffer size (kB)} \\\\
		 	\\textbf{File size (kB)}  """ + reduce(lambda a, b: a + f" & \multicolumn{{1}}{{c |}}{{{b}}}", buffer_sizes, "") + """\\\\
		 	\\hline
		 	\\hline
""" 
	
	for file_size in table_data:
		row = f"\\textbf{{{file_size}}} & "
		for buffer_size in table_data[file_size]:
			list_of_vals = table_data[file_size][buffer_size]
			avg = sum(list_of_vals) / len(list_of_vals)

			row += "%.2f & " % avg

		for _ in range(len(buffer_sizes) - len(table_data[file_size])):
			row += "{} & "

		table += row[:-2] + "\\\\\n"

	table += f"""

			\\hline

		\\end{{tabular}}
		}}
		\\label{{tbl:data_{underscore_case(f'{fs} {identifier} {report.name}')} }}
	\\end{{center}}
\\end{{table}}
	"""

	return table


def generate_tables(result: IOZoneResult, out_dir: str):
	for r in result:
		table = generate_table(r, result.fs, result.identifier)
		filename = f"{r.name}-{result.identifier}-table.tex"
		with open(f"{out_dir}/{filename}", "w+") as f:
			f.write(table)
		print(f"Generated table for {filename}")

# sniffs is a list of (identifier, sniff)
def generate_sniff_table(sniffs: List[Tuple[str, Sniff]], out_dir: str):
	table = """\\begin{table}[!ht]
	\\begin{center}
	\\caption{Network bandwidth during the benchmarks of the cloud-based filesystems}
	
	\\resizebox{\\textwidth}{!}{
		\\begin{tabular}{| r | r | r | r | r | }
			\\hline
			{} & \multicolumn{4}{c |}{Bandwidth (kbit/s)} \\\\
		 	\multicolumn{1}{| c |}{\\textbf{Filesystem}} & \multicolumn{1}{c |}{Minimum} & \multicolumn{1}{c |}{Average} & \multicolumn{1}{c |}{Median} & \multicolumn{1}{c |}{Maximum} \\\\
		 	\\hline
		 	\\hline
""" 

	for identifier, sniff in sniffs:
		kbps_list = sorted([entry.bps() / 1000 for entry in sniff.entries])

		mi = "%.2f" % min(kbps_list)
		ma = "%.2f" % max(kbps_list)
		avg = "%.2f" % average(kbps_list)
		med = "%.2f" % median(kbps_list)

		table += f"{identifier} & {mi} & {avg} & {med} & {ma}\\\\\n"


	table += """

			\\hline

		\\end{tabular}
		}
		\\label{tbl:sniff-data}
	\\end{center}
\\end{table}
"""

	with open(f"{out_dir}/sniff-table.tex", "w+") as f:
		f.write(table)

def generate_stat_tables(result: IOZoneResult, out_path: str):

	fs_identifier = f"{result.fs} ({result.identifier})"

	table = f"""
	\\begin{{table}}
	\\caption{{Joint mean and joint covariance of {fs_identifier}}}
	\\begin{{tabular}}{{| c | c | c |}}
	\hline
	{{}} & \\textbf{{Joint mean}} & \\textbf{{Joint covariance}}\\\\
	\hline
	\hline
"""	
	for report in result:
		mean, covariance = joint_distribution(report)

		mean1, mean2 = "%.2f" % mean[0], "%.2f" % mean[1]
		covariance1, covariance2 = "%.2f" % covariance[0][0], "%.2f" % covariance[0][1]
		covariance3, covariance4 = "%.2f" % covariance[1][0], "%.2f" % covariance[1][1]

		table += f"{report.name} & $\left[ \\begin{{array}}{{rr}} {mean1} & {mean2} \end{{array}}\\right] $ & $\left[ \\begin{{array}}{{rr}} {covariance1} & {covariance2} \\\\ {covariance3} & {covariance4} \end{{array}}\\right] $\\\\ \n"
		table += "{} & {} & {} \\\\ \n"

	# $Q = \left[ \\begin{array}{cc} 0.1 & 0  \\ 0 & 0.1 \end{array}\\right]$ & $Q = \left[ \begin{array}{cc} 0.01 & 0 \\ 0 & 0.01  \end{array}\\right]$\\
	# \hline
	# $R = 5 \\times 10^{-3}$ & $R = 5 \\times 10^{-5}$ \\
	table +=f"""
	\hline
	\end{{tabular}}
	\label{{tbl:stat-{underscore_case(fs_identifier)}}}
	\end{{table}}
"""
	with open(f"{out_path}/stat-table-{underscore_case(result.identifier)}.tex", "w+") as f:
		f.write(table)

def generate_bootstrap_table(fs: str, with_ubc: IOZoneResult, without_ubc: IOZoneResult, out_path: str):
	table = f"""
	\\begin{{table}}[ht!]
	\\caption{{80\\% confidence intervals of the different benchmark test performances of {fs}}}
	\\begin{{tabular}}{{| c | c | c |}}
	\hline
	{{}} & \\textbf{{UBC enabled}} & \\textbf{{UBC disabled}} \\\\
	\hline
	\hline
	"""
	
	for ubc_rep, no_ubc_rep in zip(with_ubc, without_ubc):
		row = f"{ubc_rep.name} &"
		ubc_low, ubc_high = confidence_interval(ubc_rep)

		row += f"$\left[ \\begin{{array}}{{rr}} {'%.2f' % ubc_low} & {'%.2f' % ubc_high} \end{{array}}\\right] $ &"

		no_ubc_low, no_ubc_high = confidence_interval(no_ubc_rep)

		row += f"$\left[ \\begin{{array}}{{rr}} {'%.2f' % no_ubc_low} & {'%.2f' % no_ubc_high} \end{{array}}\\right] $"

		row += "\\\\ \n"

		table += row

	table +=f"""
		\hline
		\end{{tabular}}
		\label{{tbl:bootstrap-table-{fs.lower()}}}
		\end{{table}}
	"""
	print(table)
	with open(f"{out_path}/bootstrap-table.tex", "w+") as f:
		f.write(table)
def generate_bootstrap_tables(benchmarks: List[Benchmark], out_path: str):
	
	i = 0
	while i < len(benchmarks):
		b1 = benchmarks[i]
		b2 = benchmarks[i + 1]
		while True:
			try: 
				generate_bootstrap_table(b1.fs, b1.result, b2.result, f"{out_path}/{b1.fs}")
				break
			except:
				pass

		i += 2
