from typing import List
import numpy as np
import matplotlib.pyplot as plt

from statistics import NormalDist

def confidence_interval(data, confidence=0.95):
	dist = NormalDist.from_samples(data)
	z = NormalDist().inv_cdf((1 + confidence) / 2.)
	h = dist.stdev * z / ((len(data) - 1) ** .5)
	return dist.mean - h, dist.mean + h

def create_bell(_data: List[int]):
	data = sorted(_data)

	interval = confidence_interval(data)

	print(data)

	print(interval)

	# # Plot between -10 and 10 with .001 steps.
	# x_axis = np.arange(0, len(data))
	# # Mean = 0, SD = 2.
	# plt.plot(x_axis, data)
	# plt.show()