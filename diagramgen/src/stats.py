
import numpy as np
from scipy.stats import ttest_ind
import matplotlib.pyplot as plt

import random

from typing import List, Tuple

from src.iozone_models import IOZoneReport

def cohensd(data: List[int]):

	mid = len(data) // 2

	# random.shuffle(data)

	group1 = data[:mid]
	group2 = data[mid:]

	# Perform a two-sample t-test
	t_stat, p_value = ttest_ind(group1, group2)

	# Calculate the effect size using Cohen's d
	d = (np.mean(group1) - np.mean(group2)) / np.sqrt(((len(group1) - 1) * np.var(group1, ddof=1) + (len(group2) - 1) * np.var(group2, ddof=1)) / (len(group1) + len(group2) - 2))

	print("t-statistic: ", t_stat)
	print("p-value: ", p_value)
	print("Cohen's d: ", d)


def median(data: List[int]):
	l = sorted(data)
	dl = len(l)
	return float(l[int(dl / 2)] if dl % 2 == 0 else (l[int((dl - 1)/2)] + l[int((dl + 1)/2)])/2)

def average(data: List[int]) -> float:
	return sum(data) / len(data)

def joint_distribution(report: IOZoneReport) -> Tuple[List[float], List[List[float]]]:

	def my_function(fs: int, bs: int):
		try:
			bs_values = report[fs].raw()
		except KeyError:
			return None
		
		values = [dp.value for dp in bs_values if dp.buffer_size == bs]
		return values if len(values) > 0 else None

	# Define the range of values for each parameter
	x_values = np.array([1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144])
	y_values = np.array([4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384])

	# Calculate the joint probability mass function of the two parameters
	joint_pmf = np.zeros((len(x_values), len(y_values)))
	for i, x in enumerate(x_values):
		for j, y in enumerate(y_values):
			val = my_function(x, y)
			if val is not None:  # Check if function is defined for x, y
				joint_pmf[i, j] = np.mean(val)
			else:
				joint_pmf[i, j] = np.nan
	joint_pmf /= np.nansum(joint_pmf)

	# Calculate the joint mean of the function
	joint_mean = np.array([0.0, 0.0])
	for i, x in enumerate(x_values):
		for j, y in enumerate(y_values):
			if not np.isnan(joint_pmf[i, j]):
				joint_mean += np.array([x, y]) * joint_pmf[i, j]

	# Calculate the joint covariance of the function
	joint_covariance = np.zeros((2, 2))
	for i, x in enumerate(x_values):
		for j, y in enumerate(y_values):
			if not np.isnan(joint_pmf[i, j]):
				outer_product = np.outer(np.array([x, y]) - joint_mean, np.array([x, y]) - joint_mean)
				joint_covariance += joint_pmf[i, j] * outer_product

	# print("Joint mean:", joint_mean)
	# print("Joint covariance:", joint_covariance)

	return joint_mean, joint_covariance

	# plt.figure()
	# plt.contour(x_values, y_values, joint_pmf.T)
	# plt.xlabel('X')
	# plt.ylabel('Y')
	# plt.title('Joint Probability Mass Function')
	# plt.show()

	# # Calculate the values of the function for all combinations of parameter values
	# Z = np.zeros((len(x_values), len(y_values)))
	# for i, x in enumerate(x_values):
	# 	for j, y in enumerate(y_values):
	# 		val = my_function(x, y)
	# 		if val is not None:
	# 			Z[i, j] = np.mean(val)

	# # Create a heatmap of the function values
	# fig, ax = plt.subplots()
	# im = ax.imshow(Z, cmap='jet')

	# # Set the x and y axis labels
	# ax.set_xticks(np.arange(len(y_values)))
	# ax.set_yticks(np.arange(len(x_values)))
	# ax.set_xticklabels(y_values)
	# ax.set_yticklabels(x_values)

	# # Rotate the x axis labels
	# plt.setp(ax.get_xticklabels(), rotation=45, ha="right",
	# 		rotation_mode="anchor")

	# # Add a colorbar to the heatmap
	# cbar = ax.figure.colorbar(im, ax=ax)

	# # Set the title and axis labels
	# ax.set_title("Function values")
	# ax.set_xlabel("y")
	# ax.set_ylabel("x")

	# plt.show()