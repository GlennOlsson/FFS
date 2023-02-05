from typing import List
from dataclasses import dataclass
import functools

@dataclass
class IOZoneDataPoint:
	buffer_size: int
	value: int

	def __str__(self):
		return f"{self.value} ({self.buffer_size} kB)"

	def __eq__(self, other: "IOZoneDataPoint"):
		return self.value == other.value and self.buffer_size == other.buffer_size

	def __gt__(self, other: "IOZoneDataPoint"):
		return self.value > other.value if self.value != other.value else self.buffer_size > other.buffer_size

class IOZoneFileSizeResult:
	size: int
	data: List[IOZoneDataPoint]

	def __init__(self, size: int):
		self.size = size
		self.data = []
	
	def add(self, data: IOZoneDataPoint):
		self.data.append(data)

	def __iter__(self):
		return self.data.__iter__()

	def __getitem__(self, buffer_size: int) -> IOZoneDataPoint:
		for d in self.data: 
			if d.buffer_size == buffer_size:
				return d
		raise KeyError(buffer_size)
	
	def raw(self) -> List[IOZoneDataPoint]:
		return self.data

	def raw_values(self) -> List[int]: return [point.value for point in self.data]

	def sum(self) -> int: return sum(self.raw_values())

	def avg(self):
		total = self.sum()
		return total / len(self.data)
	
	def extend(self, result: "IOZoneFileSizeResult"):
		self.data.extend(result.data)

	def __str__(self):
		res = f"{self.size}: "
		for d in self.data:
			res += f"{str(d)} "
		return res

class IOZoneReport:
	name: str # Eg Random Write
	data: List[IOZoneFileSizeResult]

	count_data_points: int

	def __init__(self, name: str):
		self.name = name
		self.data = []
		self.count_data_points = 0
	
	def add(self, data: IOZoneFileSizeResult):
		self.data.append(data)
		self.count_data_points += len(data.data)

	def __iter__(self):
		return self.data.__iter__()

	def __getitem__(self, file_size: int) -> IOZoneFileSizeResult:
		for d in self.data: 
			if d.size == file_size:
				return d
		raise KeyError(file_size)
	
	def raw_values(self) -> List[int]: return functools.reduce(lambda prev, curr: prev + curr.raw_values(), self.data, [])

	def raw(self) -> List[IOZoneDataPoint]:
		return functools.reduce(lambda prev, curr: prev + curr.raw(), self.data, [])
	
	def sum(self): return sum(self.raw_values())

	def avg(self):
		total = self.sum()
		return total / len(self.raw_values())

	def extend(self, report: "IOZoneReport"):
		for file_size_res in report.data:
			self[file_size_res.size].extend(file_size_res)
			self.count_data_points += len(file_size_res.data)

	def print(self):
		print(f"{self.name}: ")
		for d in self.data:
			print(str(d))

class IOZoneResult:
	fs: str # eg ffs
	reports: List[IOZoneReport]
	identifier: str

	def __init__(self, identifier: str, fs: str):
		self.identifier = identifier
		self.fs = fs
		self.reports = []
	
	def add(self, report: IOZoneReport):
		self.reports.append(report)
	
	def __getitem__(self, report_name: str) -> IOZoneReport:
		for r in self.reports: 
			if r.name == report_name:
				return r
		raise KeyError(report_name)
	
	def extend(self, report: "IOZoneResult"):
		for r in report.reports:
			self[r.name].extend(r)
	
	def print(self):
		print(f"{self.fs}: ")
		for report in self.reports:
			report.print()
			print()