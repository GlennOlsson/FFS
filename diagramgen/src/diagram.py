from typing import List, Dict
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.font_manager as ft
import scipy.stats as stats

from src.stats import median, average

import random

from matplotlib.ticker import ScalarFormatter

from src.iozone_models import IOZoneReport, IOZoneResult
from src.sniff_models import Sniff

def _confidence_interval(data, confidence=0.95):
	# Calculate the mean of each test and the standard error of the mean (SEM) for each test
	n = len(data[0]) # number of data points in each test
	means = [np.mean(test) for test in data]
	sems = [np.std(test) / np.sqrt(n) for test in data]

	# Choose a confidence level and calculate the critical value using a t-distribution
	alpha = 0.1 # desired confidence level
	df = 19 # degrees of freedom (number of tests - 1)
	critical_value = stats.t.ppf(1 - alpha / 2, df)

	# Calculate the lower and upper bounds of the confidence interval for each mean
	lower_bounds = [mean - sem * critical_value for mean, sem in zip(means, sems)]
	upper_bounds = [mean + sem * critical_value for mean, sem in zip(means, sems)]

	# Calculate the overall mean of all the data and the standard error of the mean for the entire dataset
	overall_mean = np.mean(means)
	overall_sem = np.std(means) / np.sqrt(20)

	# Calculate the lower and upper bounds of the confidence interval for the overall mean
	overall_lower_bound = overall_mean - overall_sem * critical_value
	overall_upper_bound = overall_mean + overall_sem * critical_value

	return lower_bounds, upper_bounds, overall_lower_bound, overall_upper_bound

def bootstrap(data, num_samples, statistic, alpha):
    """
    Draw num_samples bootstrap samples from data and compute statistic of each sample.
    Returns the bootstrapped confidence interval for the statistic.
    """
    n = len(data)
    samples = np.array([random.choices(data, k = n) for _ in range(num_samples)])
    stats = np.array([statistic(sample) for sample in samples])
    lower = np.percentile(stats, alpha / 2 * 100)
    upper = np.percentile(stats, 100 - alpha / 2 * 100)
    return lower, upper

def right_align_labels(labels: List[str]):
	longest_word = ""
	for l in labels:
		if len(l) >= len(longest_word):
			longest_word = l
	
	for i in range(len(labels)):
		word = labels[i]
		spaces = len(longest_word) - len(word)
		labels[i] = (" " * spaces) + word

def draw_histogram(report: IOZoneReport, ax: plt.Axes):
	
	labels = []
	for data in report:
		# plot the histogram with 50 bins
		ax.hist(data.raw_values(), bins=50, alpha=0.5)
		labels.append(f"{data.size} kB")

	right_align_labels(labels)

	# add labels to the x and y axis
	ax.set_xlabel('Performance, kB/s', loc="right")
	ax.set_ylabel('Frequency')

	# fig.legend(prop=ft.FontProperties(family="monospace"), loc="upper right", labels=labels, bbox_to_anchor=(1.27, 1.02), ncol=1, fancybox=True, shadow=True, title="File size")

	ax.set_title(report.name, weight=700)

	ax.tick_params(axis='x', rotation=30)

	# ax.get_xaxis().set_major_formatter(ScalarFormatter())
	ax.ticklabel_format(useOffset=False, style="plain")

	# display the histogram
	# plt.savefig(f"{out_path}/hist.pdf", bbox_inches='tight')

	return labels

def draw_histograms(report: IOZoneResult, out_dir: str):
	if report.is_empty():
		return

	cols = 2
	rows = 3

	fig, ax = plt.subplots(ncols=cols, nrows=rows, figsize=(14, 17))

	col = 0
	row = 0
	for r in report:
		labels = draw_histogram(r, ax[row][col])
		print(f"Generated Histo for {report.fs} ({report.identifier}) {r.name}")
		col += 1
		if col >= cols:
			col = 0
			row += 1

	fig.legend(prop=ft.FontProperties(family="monospace"), loc="center right", labels=labels, ncol=1, fancybox=True, shadow=True, title="File size")

	title = f"{report.fs.upper()} ({report.identifier})"

	fig.suptitle(title, weight=1000, size="xx-large", y=0.92)

	filename = f"{report.fs}-{report.identifier}-hist.pdf"
	fig.savefig(f"{out_dir}/{filename}", bbox_inches='tight')

	print(f"Saved histo for {title}")

def sniff_diagram(sniff: Sniff, out_dir: str):
	bps_per_entry = sorted([entry.bps() / 1000 for entry in sniff.entries if entry.bytes != 0])

	print(len(bps_per_entry))
	print(bps_per_entry[:5])
	fig, ax = plt.subplots()

	# ax.hist(bps_per_entry, bins=100, alpha=0.5)

	# ax.set_xlabel('Bandwidth, kB/s', loc="right")
	# ax.set_ylabel('Frequency')
	# ax.ticklabel_format(useOffset=False, style="plain")

	ax.boxplot(bps_per_entry)
	ax.set_yscale("log")

	# plt.show()

def sniff_histogram(sniff: Sniff, out_dir: str):
	if sniff.is_empty():
		return
	
	bps_per_entry = [entry.bps() / 1000 for entry in sniff.entries]

	fig, ax = plt.subplots()
	
	ax.hist(bps_per_entry, bins=30, alpha=0.5)

	ax.set_xlabel('Bandwidth, kB/s', loc="right")
	ax.set_ylabel('Frequency')
	ax.ticklabel_format(useOffset=False, style="plain")

	# ax.get_xaxis().set_major_formatter(ScalarFormatter())

	fig.savefig(f"{out_dir}/sniff-histo.pdf")
	print("Created sniff histogram")

def draw_box_plots(reports: List[IOZoneResult], out_dir: str):
	reports = [report for report in reports if not report.is_empty()]

	# Assume all has same report names
	report_names = [report.name for report in reports[0].reports]

	font_dict = {'fontsize': 20}

	for report_name in report_names:
		fig, ax = plt.subplots(figsize =(25, 9))
		ax.set_title(report_name, font_dict)

		full_data = []
		labels = []
		for i in range(len(reports)):
			fs_report = reports[i]

			full_data.append(fs_report[report_name].raw_values())

			labels.append(f"{fs_report.fs.upper()} ({fs_report.identifier})")

			data = fs_report[report_name].raw_values()
			med = median(data)
			avg = average(data)
			
			move_factor = 0.097
			start_x = 0.21
			
			fig.text(start_x + i * move_factor, 0.05, "average = %.2f kB/s" % round(avg, 2), horizontalalignment='right', size=11)
			fig.text(start_x + i * move_factor, 0.03, "median = %.2f kB/s" % round(med, 2), horizontalalignment='right', size=11)

		ax.set_yscale('log')

		ax.set_ylabel("Performance, kB/s", font_dict)

		ax.boxplot(full_data, labels=labels)
		
		ax.tick_params("x", labelsize=15)
		ax.tick_params("y", labelsize=15)

		filename = f"{report_name}-boxplot.pdf"
		
		fig.savefig(f"{out_dir}/{filename}", bbox_inches='tight')

		print(f"Generated boxplot for {filename}")



def draw_bfs_histogram(report: IOZoneReport, ax: plt.Axes):
	
	raw_data = report.raw()

	# Buffer size -> data value
	buffer_sizes: Dict[int, List[int]] = {}

	for dp in raw_data:
		if dp.buffer_size not in buffer_sizes:
			buffer_sizes[dp.buffer_size] = []
		buffer_sizes[dp.buffer_size].append(dp.value)

	labels = []
	for size in buffer_sizes.keys():
		data = buffer_sizes[size]
		# plot the histogram with 50 bins
		ax.hist(data, bins=50, alpha=0.5)
		labels.append(f"{size} kB")

	right_align_labels(labels)

	# add labels to the x and y axis
	ax.set_xlabel('Performance, kB/s', loc="right")
	ax.set_ylabel('Frequency')

	# fig.legend(prop=ft.FontProperties(family="monospace"), loc="upper right", labels=labels, bbox_to_anchor=(1.27, 1.02), ncol=1, fancybox=True, shadow=True, title="File size")

	ax.set_title(report.name, weight=700)

	ax.tick_params(axis='x', rotation=30)

	# ax.get_xaxis().set_major_formatter(ScalarFormatter())
	ax.ticklabel_format(useOffset=False, style="plain")

	# display the histogram
	# plt.savefig(f"{out_path}/hist.pdf", bbox_inches='tight')

	return labels


def draw_bfs_histograms(report: IOZoneResult, out_dir: str):
	if report.is_empty():
		return

	cols = 2
	rows = 3

	fig, ax = plt.subplots(ncols=cols, nrows=rows, figsize=(14, 17))

	col = 0
	row = 0
	for r in report:
		labels = draw_bfs_histogram(r, ax[row][col])
		print(f"Generated Buffer Size Histo for {report.fs} ({report.identifier}) {r.name}")
		col += 1
		if col >= cols:
			col = 0
			row += 1

	fig.legend(prop=ft.FontProperties(family="monospace"), loc="center right", labels=labels, ncol=1, fancybox=True, shadow=True, title="File size")

	title = f"{report.fs.upper()} ({report.identifier})"

	fig.suptitle(title, weight=1000, size="xx-large", y=0.92)

	filename = f"{report.fs}-{report.identifier}-bfs-hist.pdf"
	fig.savefig(f"{out_dir}/{filename}", bbox_inches='tight')

	print(f"Saved histo for {title}")


def draw_scatter(report: IOZoneReport, identifier: str, out_file: str):
	fig, ax = plt.subplots(dpi=100)

	scats: List[any] = []
	labels: List[str] = []

	for file_size_report in report:

		# ax = fig.add_subplot(3, 3, file_size_i + 1)

		# ax.set_title(f"File size = {y_vals[file_size_i]} kB")

		X = []
		Y = []
		for dp in file_size_report:
			X.append(dp.buffer_size)
			Y.append(dp.value)

		# ax.set_xlabel('Buffer size, kB')
		# ax.set_xscale('log', base=2)
		# ax.set_xticks(x_vals)
		# ax.get_xaxis().set_major_formatter(ScalarFormatter())

		# ax.set_ylabel('Performance, kB/s')

		# ax.tick_params(axis='x', rotation=45)

		scat = ax.scatter(X, Y, alpha=0.5)#, color=colors[file_size_i])
		scats.append(scat)
		labels.append(f"{file_size_report.size} kB")
	

	ax.set_xscale('log', base=2)
	ax.set_xticks(X)
	ax.get_xaxis().set_major_formatter(ScalarFormatter())
	ax.set_ylabel('Performance, kB/s')
	ax.set_xlabel('Buffer size, kB')
	ax.tick_params(axis='x', rotation=45)

	plt.legend(scats, labels, loc="center right", bbox_to_anchor=(1.3, 0.5), ncol=1, fancybox=True, shadow=True, title="File size")

	ax.set_title(f"{identifier} {report.name}")

	# plt.show()
	fig.savefig(out_file, bbox_inches='tight')

def draw_scatters(result: IOZoneResult, out_dir: str):
	for report in result:
		print(report.name)

		draw_scatter(report, f"{result.fs} ({result.identifier})" ,f"{out_dir}/scatter-{result.identifier}-{report.name}.pdf")
		