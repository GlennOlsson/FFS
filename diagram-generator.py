from functools import reduce
from typing import List, Tuple
from matplotlib.axes import Axes
from matplotlib.figure import Figure
from mpl_toolkits import mplot3d
import matplotlib.pyplot as plt
import numpy as np
import re
from matplotlib import cm
from matplotlib.ticker import LinearLocator, ScalarFormatter
import os
import math

files = {
	"ffs": "FFS",
	"fejk-ffs": "FFFS",
	"gcsf": "GCSF",
	"local": "APFS"
}

fig_output_location = "../doc/figures/benchmarking"
# fig_output_location = "./"

def output_path(fig_name: str, fs: str):
	return f"{fig_output_location}/{fs}/{fig_name}"

if not os.path.exists(fig_output_location):
	os.mkdir(fig_output_location)

for fs in files.keys():
	path = output_path("", fs)
	if not os.path.exists(path):
		os.mkdir(path)

report_pattern = r"\"(.+) report\""

def underscore_case(s: str):
	return s.replace(" ", "_").lower()

def generate_table(test_name: str, fs: str, x_vals: List[int], y_vals: List[int], z_vals: List[List[int]]):
	columns = len(x_vals)

	rows_str = "\n"
	for row_i in range(len(y_vals)):
		row = y_vals[row_i]
		# Add each z value for the row, and append with empty {} for each None value so each table has the same amount of rows
		rows_str += "\t\t\t\\textbf{" + str(row) + "} " + reduce(lambda a, b: a + " & " + str(b), z_vals[row_i], "") + reduce(lambda a, b: a + " & " + str(b), ["{}"] * (len(x_vals) - len(z_vals[row_i])), "") + "\\\\\n"

	fs_name = files[fs]

	content ="""\\begin{table}[!ht]
	\\begin{center}
		\\caption{IOZone result for the """ + test_name + """ test on """ + fs_name + """ in kilobytes per second}
		\\resizebox{\\textwidth}{!}{\\begin{tabular}{| r """ + "| r " * columns + """| }
			
			\\hline
			{} & \multicolumn{"""+ str(len(x_vals)) +"""}{c |}{Buffer size (kB)} \\\\
			\\textbf{File size (kB)}  """ + reduce(lambda a, b: a + f" & \multicolumn{{1}}{{c |}}{{{b}}}", x_vals, "") + """\\\\
			\\hline
			\\hline""" + rows_str + """

			\\hline

		\\end{tabular}}
		\\label{tbl:data_""" + underscore_case(fs + " " + test_name) + """}
	\\end{center}
\\end{table}
	"""

	with open(f"{output_path(test_name, fs)}.tex", "w") as f:
		f.write(content)

def generate_graphs(test_name: str, fs: str, x_vals: List[int], y_vals: List[int], z_vals: List[List[int]]):
	fig, ax = plt.subplots(dpi=100)

	limit = 9 if fs != "gcsf" else 8

	scats: List[any] = []
	labels: List[str] = []

	for file_size_i in range(limit):

		# ax = fig.add_subplot(3, 3, file_size_i + 1)

		# ax.set_title(f"File size = {y_vals[file_size_i]} kB")

		X = []
		Y = []
		row = z_vals[file_size_i]
		for j in range(len(row)):
			X.append(x_vals[j])
			Y.append(row[j])

		# ax.set_xlabel('Buffer size, kB')
		# ax.set_xscale('log', base=2)
		# ax.set_xticks(x_vals)
		# ax.get_xaxis().set_major_formatter(ScalarFormatter())

		# ax.set_ylabel('Performance, kB/s')

		# ax.tick_params(axis='x', rotation=45)

		scat = ax.scatter(X, Y, alpha=0.5)#, color=colors[file_size_i])
		scats.append(scat)
		labels.append(f"{y_vals[file_size_i]} kB")
	

	ax.set_xscale('log', base=2)
	ax.set_xticks(x_vals)
	ax.get_xaxis().set_major_formatter(ScalarFormatter())
	ax.set_ylabel('Performance, kB/s')
	ax.set_xlabel('Buffer size, kB')
	ax.tick_params(axis='x', rotation=45)

	plt.legend(scats, labels, loc="center right", bbox_to_anchor=(1.3, 0.5), ncol=1, fancybox=True, shadow=True, title="File size")

	# plt.show()
	fig.savefig(f"{output_path(test_name, fs)}.pdf", bbox_inches='tight')

def parse_report(lines: List[str], test_name, fs: str, index: int) -> Tuple[int, List[List[int]]]:
	rec_lens_line = lines[index]
	# Split by whitespace and convert to numbers. First and last char of str is "
	rec_lens = list(map(lambda s: int(s[1:-1]), rec_lens_line.split()))

	file_sizes: List[int] = []

	values: List[List[int]] = []

	i = index + 1
	while i < len(lines) and lines[i] != "":
		l = lines[i].split()
		# Strip first and last " and convert to int
		file_sizes.append(int(l[0][1:-1]))

		# All but the file size
		values.append(list(map(lambda s: int(s), l[1:])))

		i += 1

	print("Generating graphs for ", test_name, fs)

	generate_graphs(test_name, fs, rec_lens, file_sizes, values)
	generate_table(test_name, fs, rec_lens, file_sizes, values)

	return (i, values)

def parse_file(fs_name: str, lines: List[str]):
	i = 0

	# Report name, and the values of the report
	reports: dict[str, List[List[str]]] = {}
	
	while i < len(lines):
		l = lines[i]
		
		m = re.match(report_pattern, l)
		if m is not None:
			report_name = m.group(1)
			print(f"'{report_name}'")
			i, report_vals = parse_report(lines, report_name, fs_name, i + 1)

			reports[report_name] = report_vals

		i += 1
	
	return reports


bench_reports: dict[str, dict[str, List[List[str]]]] = {}
test_names = [
	"Write",
	"Re-Write",
	"Read",
	"Re-Read",
	"Random read",
	"Random write"
]

for filename, fs in files.items():
	
	file_path = filename + '.log'
	with open(file_path) as f:
		file_content = f.read()

	file_content = file_content.replace("Reader", "Read")
	file_content = re.sub(re.compile(r"[Ww]riter") , "Write", file_content)

	seek_str = "iozone test complete.\nExcel output is below:\n"
	seek_location = file_content.find(seek_str)

	file_content = file_content[seek_location + len(seek_str):]

	lines = file_content.splitlines()

	reports = parse_file(filename, lines)

	bench_reports[fs] = reports

def median(data: List[int]):
	l = sorted(data)
	dl = len(l)
	return round(l[int(dl / 2)] if dl % 2 == 0 else (l[int((dl - 1)/2)] + l[int((dl + 1)/2)])/2)

def average(data: List[int]):
	return round(sum(data) / len(data))

# Generate boxplot for each test, for all 4 filesystems
for test in test_names:

	all_data = []

	for fs in files.values():
		fs_test_report = bench_reports[fs][test]
		data = sorted([int(item) for l in fs_test_report for item in l])
		all_data.append(data)

	
	fig, ax = plt.subplots()

	fig.set_size_inches(8, 6)

	ax.boxplot(all_data, labels=files.values())

	for i, data in zip(range(len(all_data)), all_data):
		med = median(data)
		avg = average(data)

		ax.text(i + 1.35, -0.1, f"median = {round(med, 2)} kB/s", transform=ax.get_xaxis_transform(),
             horizontalalignment='right', size='x-small',)
		ax.text(i + 1.35, -0.13, f"average = {round(avg, 2)} kB/s", transform=ax.get_xaxis_transform(),
             horizontalalignment='right', size='x-small',)


	ax.set_title(test)

	ax.set_yscale('log')

	ax.set_ylabel("Performance, kB/s")

	fig.savefig(f"{fig_output_location}/{test}_box.pdf")

