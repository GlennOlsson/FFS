from cgi import parse
from typing import List
from mpl_toolkits import mplot3d
import matplotlib.pyplot as plt
import numpy as np
import re
from matplotlib import cm
from matplotlib.ticker import LinearLocator

file_path = 'ffs.log'
with open(file_path) as f:
	file_content = f.read()

seek_str = "iozone test complete.\nExcel output is below:\n"
seek_location = file_content.find(seek_str)

file_content = file_content[seek_location + len(seek_str):]

# print(file_content)

report_pattern = r"\"(.+) report\""

lines = file_content.splitlines()

def generate_graph(name: str, x_vals: List[int], y_vals: List[int], z_vals: List[List[int]]):
	ax = plt.axes(projection='3d')

	# Find max value of z list
	m = 0
	for l in z_vals:
		for v in l:
			if v > m:
				m = v

	# Data for a three-dimensional line
	# 0 to 110% of max of z values
	zline = np.linspace(0, m*1.1)
	xline = np.linspace(0, max(x_vals) * 1.1)
	yline = np.linspace(0, max(y_vals) * 1.1)
	ax.plot3D(xline, yline, zline, 'gray')

	# zdata = 15 * np.random.random(100)
	# xdata = np.sin(zdata) + 0.1 * np.random.randn(100)
	# ydata = np.cos(zdata) + 0.1 * np.random.randn(100)
	ax.contour3D(x_vals, y_vals, z_vals, c=z_vals, cmap='Greens');

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

	generate_graph(name, rec_lens, file_sizes, values)

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

x_vals = [4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384]
y_vals = [1024, 2048, 4096, 8192, 16384]
z_vals = [
	[71, 170, 169, 93, 77, 130, 74, 133, 157],
	[183, 72, 150, 153, 216, 96, 67, 86, 32, 174],
	[132, 105, 76, 112, 166, 131, 136, 87, 172, 101, 105],
	[255, 337, 223, 275, 316, 193, 188, 253, 177, 169, 174, 239],
	[320, 218, 288, 191, 291, 254, 363, 269, 340, 364, 317, 252, 208]
]

X = []
Y = []
Z = []
for i in range(2, len(z_vals)):
	row = z_vals[i]
	for j in range(len(row)):
		Y.append(y_vals[i])
		X.append(x_vals[j])
		Z.append(row[j])
	
	break


fig, ax = plt.subplots()

ax.set_xlabel('Record length, kB')
ax.set_xticks(x_vals)

ax.set_ylabel('Performance, kB/s')

surf = ax.scatter(X, Z, color="black")

# fig, ax = plt.subplots(subplot_kw={"projection": "3d"})

# ax.set_xlabel('Record length, kB')
# ax.set_xticks(x_vals)

# ax.set_ylabel('File size, kB')
# ax.set_yticks(y_vals)

# ax.set_zlabel('Performance, kB/s')

# surf = ax.scatter3D(X, Y, Z, color="black")


plt.show()