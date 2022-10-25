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

report = "gcsf"
title = "GCSF"

fig_output_location = "../doc/figures/benchmarking/" + report

if not os.path.exists(fig_output_location):
	os.mkdir(fig_output_location)

file_path = report + '.log'
with open(file_path) as f:
	file_content = f.read()

file_content = file_content.replace("Reader", "Read")
file_content = re.sub(re.compile(r"[Ww]riter") , "Write", file_content)

seek_str = "iozone test complete.\nExcel output is below:\n"
seek_location = file_content.find(seek_str)

file_content = file_content[seek_location + len(seek_str):]

report_pattern = r"\"(.+) report\""

lines = file_content.splitlines()

def underscore_case(s: str):
	return s.replace(" ", "_").lower()

def generate_table(name: str, x_vals: List[int], y_vals: List[int], z_vals: List[List[int]]):
	columns = len(x_vals)

	rows_str = "\n"
	for row_i in range(len(y_vals)):
		row = y_vals[row_i]
		# Add each z value for the row, and append with empty {} for each None value so each table has the same amount of rows
		rows_str += "\t\t\t\\textbf{" + str(row) + "} " + reduce(lambda a, b: a + " & " + str(b), z_vals[row_i], "") + reduce(lambda a, b: a + " & " + str(b), ["{}"] * (len(x_vals) - len(z_vals[row_i])), "") + "\\\\\n"


	content ="""\\begin{table}[!ht]
	\\begin{center}
		\\caption{IOZone result for """ + title + """ """ + name + """}
		\\resizebox{\\textwidth}{!}{\\begin{tabular}{| c """ + "| c " * columns + """| }
			
			\\hline
			{} & \multicolumn{"""+ str(len(x_vals)) +"""}{c |}{Buffer size (kB)} \\\\
			\\textbf{File size (kB)}  """ + reduce(lambda a, b: a + " & " + str(b), x_vals, "") + """\\\\
			\\hline
			\\hline""" + rows_str + """

			\\hline

		\\end{tabular}}
		\\label{tbl:data_""" + underscore_case(name + " " + title) + """}
	\\end{center}
\\end{table}
	"""

	with open(f"{fig_output_location}/{name}.tex", "w") as f:
		f.write(content)

def generate_boxplot(name: str, z_vals: List[List[int]], ax: Axes):
	all_data = [item for l in z_vals for item in l]

	ax.boxplot(all_data, vert = 0)

	# ax.get_xaxis().tick_bottom()
	# ax.get_yaxis().tick_left()

	ax.set_ylabel(name)
	plt.tick_params(left=False, labelleft=False)

	ax.set_xlabel("Performance, kB/s")

	ax.get_yaxis().set_major_formatter(ScalarFormatter())
	ax.ticklabel_format(useOffset=False, style="plain")

def generate_boxplots(reports: List[Tuple[str, List[List[str]]]]):
	fig = plt.figure(num=title, figsize=(14,15), dpi=180)
	
	fig.suptitle(title, fontsize=16)

	fig_nr = 0
	for name, values in reports:
		ax = fig.add_subplot(6, 1, fig_nr + 1)

		generate_boxplot(name, values, ax)

		fig_nr += 1

	
	fig.tight_layout(rect=[0, 0.01, 1, 0.99])
	fig.savefig(f"{fig_output_location}/{title}-box.pdf")

def generate_graphs(name: str, x_vals: List[int], y_vals: List[int], z_vals: List[List[int]]):
	fig = plt.figure(num=name, figsize=(16,18), dpi=180)

	for fignr in range(5):
		ax = fig.add_subplot(3, 2, fignr + 1)

		ax.set_title(f"File size = {y_vals[fignr]} kB")

		X = []
		Y = []
		row = z_vals[fignr]
		for j in range(len(row)):
			X.append(x_vals[j])
			Y.append(row[j])

		ax.set_xlabel('Record length, kB')
		ax.set_xscale('log', base=2)
		ax.set_xticks(x_vals)
		ax.get_xaxis().set_major_formatter(ScalarFormatter())

		ax.set_ylabel('Performance, kB/s')

		ax.scatter(X, Y, color="black")
	
	fig.savefig(f"{fig_output_location}/{name}.pdf", bbox_inches='tight')

def parse_report(name: str, index: int) -> Tuple[int, List[List[int]]]:
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

	generate_graphs(name, rec_lens, file_sizes, values)
	generate_table(name, rec_lens, file_sizes, values)

	return (i, values)

def parse_file():
	i = 0

	# Report name, and the values of the report
	reports: List[Tuple[str, List[List[str]]]] = []
	
	while i < len(lines):
		l = lines[i]
		
		m = re.match(report_pattern, l)
		if m is not None:
			report_name = m.group(1)
			print(report_name)
			i, report_vals = parse_report(report_name, i + 1)

			reports.append((report_name, report_vals))
		i += 1
	
	generate_boxplots(reports)

parse_file()