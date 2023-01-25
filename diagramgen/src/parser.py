from typing import List

import re
import os

from diagramgen.src.models import IOZoneDataPoint, IOZoneFileSizeResult, IOZoneReport, IOZoneResult

def _parse_report(lines: List[str], index: int, report: IOZoneReport) -> int:
	rec_lens_line = lines[index]
	# Split by whitespace and convert to numbers. First and last char of str is "
	rec_lens = [int(s[1:-1]) for s in rec_lens_line.split()]

	i = index + 1
	while i < len(lines) and lines[i] != "":
		l = lines[i].split()

		# Strip first and last " and convert to int
		file_size = int(l[0][1:-1])

		file_size_result = IOZoneFileSizeResult(file_size)

		# All but the file size
		# One less as the first one is the file size
		for j in range(len(l) - 1):
			rec_len = rec_lens[j]
			# Add 1 as the first one was the file size
			value = int(l[j + 1])

			data_point = IOZoneDataPoint(buffer_size=rec_len, value=value)

			file_size_result.add(data_point)

		report.add(file_size_result)

		i += 1

	return i
	
report_pattern = r"\"(.+) report\""

def parse_file(path: str, identifier: str, fs: str) -> IOZoneResult:
	with open(path) as f:
		file_content = f.read()

	file_content = file_content.replace("Reader", "Read")
	file_content = re.sub(re.compile(r"[Ww]riter") , "Write", file_content)

	seek_str = "iozone test complete.\nExcel output is below:\n"
	seek_location = file_content.find(seek_str)

	file_content = file_content[seek_location + len(seek_str):]

	lines = file_content.splitlines()

	i = 0

	result = IOZoneResult(identifier, fs)
	
	while i < len(lines):
		l = lines[i]
		
		m = re.match(report_pattern, l)
		if m is not None:
			report_name = m.group(1)

			report = IOZoneReport(report_name)

			i = _parse_report(lines, i + 1, report)

			result.add(report)

		i += 1
	
	return result

def parse_files(path: str, prefix: str) -> List[IOZoneResult]:
	files = [file for file in os.listdir(path) if file.startswith(prefix)]

	print(f"Parsing {len(files)} files")

	results: List[IOZoneResult] = []
	for file in files:
		result = parse_file(f"{path}/{file}", file, fs="ffs")
		results.append(result)

	return results

