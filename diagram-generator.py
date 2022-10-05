from cgi import parse
from typing import List
from mpl_toolkits import mplot3d
import matplotlib.pyplot as plt
import numpy as np
import re
from matplotlib import cm
from matplotlib.ticker import LinearLocator, ScalarFormatter

fig_output_location = "figs/ffs"

file_path = 'ffs.log'
with open(file_path) as f:
	file_content = f.read()

seek_str = "iozone test complete.\nExcel output is below:\n"
seek_location = file_content.find(seek_str)

file_content = file_content[seek_location + len(seek_str):]

# print(file_content)

report_pattern = r"\"(.+) report\""

lines = file_content.splitlines()

def generate_graphs(name: str, x_vals: List[int], y_vals: List[int], z_vals: List[List[int]]):
	ax = plt.axes(projection='3d')

	fig = plt.figure(num=name, figsize=(16,10), dpi=180)

	for fignr in range(5):
		ax = fig.add_subplot(3, 2, fignr + 1)

		X = []
		Y = []
		Z = []
		row = z_vals[fignr]
		for j in range(len(row)):
			Y.append(y_vals[fignr])
			X.append(x_vals[j])
			Z.append(row[j])

		ax.set_xlabel('Record length, kB')
		ax.set_xscale('log', base=2)
		ax.set_xticks(x_vals)
		ax.get_xaxis().set_major_formatter(ScalarFormatter())
		ax.set_ylabel('Performance, kB/s')

		ax.scatter(X, Z, color="black")
	
	fig.savefig(f"{fig_output_location}/{name}.pdf", bbox_inches='tight')

def parse_report(name: str, index: int) -> int:
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

	print(len(values), len(values[-1]))
	print(len(file_sizes), len(rec_lens))

	generate_graphs(name, rec_lens, file_sizes, values)

	return i

def parse_file():
	i = 0
	while i < len(lines):
		l = lines[i]
		
		m = re.match(report_pattern, l)
		if m is not None:
			report_name = m.group(1)
			print(report_name)
			i = parse_report(report_name, i + 1)

		i += 1

parse_file()

# x_vals = [4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384]
# y_vals = [1024, 2048, 4096, 8192, 16384]
# z_vals = [
# 	[71, 170, 169, 93, 77, 130, 74, 133, 157],
# 	[183, 72, 150, 153, 216, 96, 67, 86, 32, 174],
# 	[132, 105, 76, 112, 166, 131, 136, 87, 172, 101, 105],
# 	[255, 337, 223, 275, 316, 193, 188, 253, 177, 169, 174, 239],
# 	[320, 218, 288, 191, 291, 254, 363, 269, 340, 364, 317, 252, 208]
# ]
# fig = plt.figure(num="Random write", figsize=(16,10), dpi=180)
# generate_figure(0, fig)
# generate_figure(1, fig)
# generate_figure(2, fig)
# generate_figure(3, fig)
# generate_figure(4, fig)

# fig.savefig(fig, bbox_inches='tight')
# generate_figure(0)

# fig, ax = plt.subplots(subplot_kw={"projection": "3d"})

# ax.set_xlabel('Record length, kB')
# ax.set_xticks(x_vals)

# ax.set_ylabel('File size, kB')
# ax.set_yticks(y_vals)

# ax.set_zlabel('Performance, kB/s')

# surf = ax.scatter3D(X, Y, Z, color="black")


# plt.show()