
import numpy as np
from scipy.stats import ttest_ind, t, anderson, norm
import matplotlib.pyplot as plt

import pandas as pd
import statsmodels.api as sm

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

def create_function(report: IOZoneReport):
	def my_function(fs: int, bs: int):
		try:
			bs_values = report[fs].raw()
		except KeyError:
			return None
		
		values = [dp.value for dp in bs_values if dp.buffer_size == bs]
		# Check if function is defined for x, y
		return max(values) if len(values) > 0 else None
	return my_function

def joint_distribution(report: IOZoneReport) -> Tuple[List[float], List[List[float]]]:

	my_function = create_function(report)

	# Define the range of values for each parameter
	x_values = np.array([1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144])
	y_values = np.array([4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384])

	# Calculate the joint probability mass function of the two parameters
	joint_pmf = np.zeros((len(x_values), len(y_values)))
	for i, x in enumerate(x_values):
		for j, y in enumerate(y_values):
			val = my_function(x, y)
			joint_pmf[i, j] = val if val is not None else np.nan
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

def confidence_interval(report: IOZoneReport):

	function = create_function(report)

	x = [1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144]
	y = [4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384]

	# Define the number of bootstrap replicates
	n_replicates = 20

	data = report.raw_values()

	# Generate bootstrap replicates
	g_replicates = np.random.choice(data, size=n_replicates)
	# for i in range(n_replicates):
		# choices = min(len(x), len(y))

		# x_v = np.random.choice(x)
		# y_v = np.random.choice(y)

		
		# val = function(x_v, y_v)
		# if val is not None:
		# 	if val < 0:
		# 		print(val)
		# 	g_replicates.append(val)

	# Calculate the mean and standard error of the bootstrap replicates
	mean_g = np.mean(g_replicates)
	se_g = np.std(g_replicates, ddof=1)

	conf_level = 0.80
	# Calculate the critical z-score for the desired confidence level
	z_critical = norm.ppf((1 + conf_level) / 2)

	# Calculate the 95% confidence interval for the true value of g
	ci_lower = mean_g - z_critical * se_g
	ci_upper = mean_g + z_critical * se_g

	if ci_lower < 0:
		raise ValueError()

	print(f'{conf_level*100}% Confidence Interval for g:', (ci_lower, ci_upper))
	return ci_lower, ci_upper