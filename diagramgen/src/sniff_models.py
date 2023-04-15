from typing import List
from dataclasses import dataclass

@dataclass
class SniffEntry:
	duration: float
	frames: int
	bytes: int

	def bps(self):
		return self.bytes / self.duration

class Sniff:
	entries: List[SniffEntry]

	total_bytes: int
	total_frames: int
	total_duration: float

	def __init__(self):
		self.entries = []
		self.total_bytes = 0
		self.total_frames = 0
		self.total_duration = 0

	def add_entry(self, entry: SniffEntry):
		self.entries.append(entry)
		self.total_bytes += entry.bytes
		self.total_frames += entry.frames
		self.total_duration += entry.duration

	def join(self, other: "Sniff"):
		for e in other.entries:
			self.add_entry(e)

	def is_empty(self):
		return len(self.entries) == 0

	def __getitem__(self, index: int):
		return self.entries[index]
	
	def avg_bps(self):
		return self.total_bytes / self.total_duration

class Expression:
	DURATION = r"Duration:\s*(\d+(\.\s*\d+)?)"
	INTERVAL = r"Interval:\s*(\d+)"
	ROW = r"(\d+)\s*<>\s*(\d+|Dur)\s*\|\s*(\d+)\s*\|\s*(\d+)"
	BORDER = r"======================"
	DUR = "Dur"