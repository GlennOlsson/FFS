from typing import List
from dataclasses import dataclass

import re

@dataclass
class SniffEntry:
	duration: float
	frames: int
	bytes: int

	def bps(self):
		return self.bytes / self.duration

class Sniff:
	entries: List[SniffEntry]

	def __init__(self):
		self.entries = []

	def add_entry(self, entry: SniffEntry):
		self.entries.append(entry)

	def join(self, other: "Sniff"):
		self.entries.extend(other.entries)

	def __getitem__(self, index: int):
		return self.entries[index]

class Expression:
	DURATION = r"Duration:\s*(\d+(\.\d+))"
	INTERVAL = r"Interval:\s*(\d+)"
	ROW = r"(\d+)\s*<>\s*(\d+|Dur)\s*\|\s*(\d+)\s*\|\s*(\d+)"
	BORDER = r"======================"
	DUR = "Dur"

def parse_box(box: str) -> Sniff:
	dur_match = re.search(Expression.DURATION, box, flags=re.M)
	duration = float(dur_match.group(1))

	row_matches = re.findall(Expression.ROW, box)
	
	s = Sniff()

	for m in row_matches:
		start, end, frames, bytes = m

		sniff_duration: float
		if end != "Dur":
			sniff_duration = float(int(end) - int(start))
		else:
			sniff_duration = round(duration - int(start), 3)
		
		entry = SniffEntry(
			duration=sniff_duration,
			frames=int(frames),
			bytes=int(bytes)
		)

		s.add_entry(entry)

	return s

def parse_boxes(content: str) -> List[Sniff]:
	lines = content.splitlines()

	start_index = None

	sniffs: List[Sniff] = []

	for i in range(len(lines)):
		l = lines[i]

		if re.match(Expression.BORDER, l) is not None:
			# If is none, top border
			if start_index is None:
				start_index = i
			else: # Else, bottom border. Parse box between
				part_box = "\n".join(lines[start_index:i])

				start_index = None

				s = parse_box(part_box)
				sniffs.append(s)

	return sniffs

def read_file(path) -> str:
	with open(path) as f:
		return f.read()

def parse(path: str) -> Sniff:
	content = read_file(path)
	
	sniffs = parse_boxes(content)
	main_sniff = sniffs[0]
	for s in sniffs[1:]:
		main_sniff.join(s)
	
	return main_sniff