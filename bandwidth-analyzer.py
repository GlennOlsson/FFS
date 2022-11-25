import csv
import typing
import re

values: typing.Dict[str, typing.List[str]] = {}

count_rows = 0

headers: typing.List[str] = []
with open('../Benchmarks/gcsf-to-134mb.csv', newline='') as file:
	content = csv.reader(file, delimiter=',', quotechar='|')
	count_rows = 0
	for c in content:
		for col_i in range(len(c)):
			col = c[col_i] # Is header if first, else a data cell
			if count_rows == 0:
				values[col] = []
				headers.append(col)
			else:
				header = headers[col_i]
				values[header].append(col)
			
		count_rows += 1

# Sample Time
# Interval
# Connection Serial Number
# Owner
# Network Interface
# Protocol
# Local
# Remote
# Connection
# Packets In
# Bytes In
# Packets Out
# Bytes Out
# Dups Received
# Out-of-order
# Retransmitted
# Min Round Trip
# Avg Round Trip

print(headers)

values["Avg Round Trip"] = list(map(lambda r: int(r[:len(r) - 3]), values["Avg Round Trip"]))

pattern = r"((?:\d+ )*\d+(?:\.\d+)?) ((?:[MK]iB)|(?:Bytes))"
# Eg. 4.59 KiB ==> 12'000
def bytes_of(val: str):
	matches = re.match(pattern, val)

	count = float(matches.group(1).replace(" ", ""))
	suffix = matches.group(2)

	if suffix == "KiB":
		return count * 1024
	elif suffix == "MiB":
		return count * 1048576
	elif suffix == "Bytes":
		return count
	else:
		print(val, suffix, "did not match suffixes")
		raise "MF"

for row in range(count_rows - 1):
	# -1 because row == 0 is the headers, not in the list

	packets_in = int(values["Packets In"][row].replace(" ", ""))
	bytes_in = bytes_of(values["Bytes In"][row])

	packets_out = int(values["Packets Out"][row].replace(" ", ""))
	bytes_out = bytes_of(values["Bytes Out"][row])

	avg_rtt = values["Avg Round Trip"][row]

	if packets_in != 0:
		avg_bytes_per_packet = bytes_in / packets_in

		s = avg_rtt / 1_000_000_000
		print(avg_bytes_per_packet, s)
		bandwidth_in = (avg_bytes_per_packet / s) # bytes / s
		mbps = bandwidth_in #* 125000
		print("In: ", mbps)
	
	if packets_out != 0:
		elapsed_out = packets_out * avg_rtt
		bandwidth_out = (bytes_out / elapsed_out) * 1_000_000_000
		mbps = bandwidth_out * 125000
		print("Out: ", mbps)
