from src.iozone_models import IOZoneDataPoint, IOZoneFileSizeResult, IOZoneReport, IOZoneResult
import random

import unittest

from typing import List , Dict

def dp():
	return IOZoneDataPoint(
		buffer_size=random.randint(1, 10_000),
		value=random.randint(200, 200_000)
	)

def file_size_result(size: int):
	res = IOZoneFileSizeResult(size)

	for _ in range(15):
		res.add(dp())
	
	return res

def report(name: str):
	report = IOZoneReport(name)

	for i in range(10):
		report.add(file_size_result(2**i))
	
	return report

result_names = [
	"Write",
	"Re-Write",
	"Read",
	"Re-Read",
	"Random Write",
	"Random Read"
]

def result():
	result = IOZoneResult("test", "test-fs")
	for name in result_names:
		result.add(report(name))

	return result

class TestIOZoneModels(unittest.TestCase):
    
	def setUp(self):
		random.seed(0)	

	def test_extend_file_size_result(self):
		r1 = file_size_result(10)
		r2 = file_size_result(10)

		expected_size = len(r1.data) + len(r2.data)

		r1.extend(r2)

		self.assertEqual(len(r1.data), expected_size)

		r1_raw = r1.raw()

		random.seed(0)
		for i in range(len(r1_raw)):
			d = r1_raw[i]
			self.assertEqual(d, dp(), f"Failed for index {i}")
	
	def test_extend_report(self):
		r1 = report("R1")
		r2 = report("R2")

		r1_raw_len = len(r1.raw_values())
		r2_raw_len = len(r2.raw_values())

		expected_size = r1_raw_len + r2_raw_len

		r1.extend(r2)

		data_points = r1.raw()

		self.assertEqual(len(data_points), expected_size)

		# Generate as many data pints and compare
		random.seed(0)
		expected = []
		for _ in range(len(data_points)):
			expected.append(dp())
		
		self.assertListEqual(sorted(data_points), sorted(expected))
	
	def test_extend_results(self):
		r1 = result()
		r2 = result()

		# Expected size per report name
		expected_size: Dict[str, int] = {}
		raw_1_lists: Dict[str, List[IOZoneDataPoint]] = {}
		raw_2_lists: Dict[str, List[IOZoneDataPoint]] = {}

		for name in result_names:
			raw_1 = r1[name].raw()
			raw_2 = r2[name].raw()
			expected_size[name] = len(raw_1) + len(raw_2)

			raw_1_lists[name] = raw_1
			raw_2_lists[name] = raw_2
		
		r1.extend(r2)

		for name in result_names:
			size = len(r1[name].raw_values())
			self.assertEqual(size, expected_size[name])
		
			data_points = r1[name].raw()

			concatenated = raw_1_lists[name] + raw_2_lists[name]
			for d in data_points:
				self.assertIn(d, concatenated)
				concatenated.remove(d)
			
			self.assertEqual(len(concatenated), 0)
			del raw_1_lists[name]
			del raw_2_lists[name]
		
		self.assertEqual(len(raw_1_lists), 0)
		self.assertEqual(len(raw_2_lists), 0)