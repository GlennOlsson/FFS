from typing import List
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm, wilcoxon
import scipy.stats as stats

import statsmodels.api as sm
import pylab
import random
from statsmodels.stats.power import TTestIndPower

from statistics import NormalDist

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

def draw_histogram(data: List[List[int]]):
	data_1d = [d for l in data for d in l]
	print(data_1d, len(data_1d))
	# plot the histogram with 50 bins
	plt.hist(data_1d, bins=50)

	# add labels to the x and y axis
	plt.xlabel('Value')
	plt.ylabel('Frequency')

	# display the histogram
	plt.show()

def create_bell(data: List[List[int]]):
	# data = sorted(_data)

	# interval = _confidence_interval(data)

	print(data)

	# statistic, p_value = wilcoxon(data[0], data[1])
	# print(statistic, p_value)

	# print(interval)

	# effect_size = 0.8
	# alpha = 0.05
	# power = 0.8

	# # Perform the power analysis
	# analysis = TTestIndPower()
	# sample_size = analysis.solve_power(effect_size=effect_size, alpha=alpha, power=power)

	# print("Sample size needed: ", sample_size)

	# data = np.array(data)
	# plt.hist(data, bins=30, edgecolor='black', alpha=0.7)

	# # Add labels and title to the plot
	# plt.xlabel("Value")
	# plt.ylabel("Frequency")
	# plt.title("Histogram of Values")

	# # Show the plot
	# plt.show()

	# # Calculate and print the distribution of values
	# mean = np.mean(data)
	# std = np.std(data)

	# print("Mean: ", mean)
	# print("Standard deviation: ", std)
	# print("Minimum value: ", np.min(data))
	# print("Maximum value: ", np.max(data))

	# confidence_interval = norm.interval(0.95, loc=mean, scale=std / np.sqrt(len(data)))
	# print("95% Confidence interval: ", confidence_interval)

	#######

	# draw_histogram(data)

	# l, u, ol, ou = _confidence_interval(data)

	# print(l, u)

	# print(ol, ou)

	# data = [d for l in data for d in l]

	# print(sorted(data))

	for d in data:

		mu, std = norm.fit(d)

		# Plot the histogram.
		plt.hist(d, bins=int(len(d) / 5), density=True, alpha=0.6, color='g')

		# Plot the PDF.
		xmin, xmax = plt.xlim()
		x = np.linspace(xmin, xmax, 100)
		p = norm.pdf(x, mu, std)
		plt.plot(x, p, 'k', linewidth=2)
		title = "Fit results: mu = %.2f,  std = %.2f" % (mu, std)
		plt.title(title)

		plt.show()

	########

	# # Plot between -10 and 10 with .001 steps.
	# x_axis = np.arange(0, len(data))
	# # Mean = 0, SD = 2.
	# plt.plot(x_axis, data)
	# plt.show()