from src.iozone_models import IOZoneResult, IOZoneReport

from typing import Dict, List

from functools import reduce

def underscore_case(s: str):
	return s.replace(" ", "_").lower()

def generate_table(report: IOZoneReport, fs: str, identifier: str) -> str:
	table = f"""\\begin{{table}}[!ht]
	\\begin{{center}}
	\\caption{{Average IOZone result for the {report.name} ({identifier}) test on {fs.upper()} in kilobytes per second}}"""
	
	# table_data[file_size][rec_len] is a list of values. These will later be averaged
	table_data: Dict[int, Dict[int, List[int]]] = {}

	for file_size_report in report:
		file_size = file_size_report.size
		if file_size not in table_data:
			table_data[file_size] = {}
		
		for dp in file_size_report:
			buffer_size = dp.buffer_size
			if buffer_size not in table_data[file_size]:
				table_data[file_size][buffer_size] = []
			
			table_data[file_size][buffer_size].append(dp.value)

	# List of buffer sizes is the maximum file size's total buffer sizes
	biggest_file_size = max(table_data.keys())
	buffer_sizes = table_data[biggest_file_size].keys()

	table += """\\resizebox{\\textwidth}{!}{\\begin{tabular}{| r """ + "| r " * len(buffer_sizes) + """| }
			\\hline
			{} & \multicolumn{"""+ str(len(buffer_sizes)) +"""}{c |}{Buffer size (kB)} \\\\
		 	\\textbf{File size (kB)}  """ + reduce(lambda a, b: a + f" & \multicolumn{{1}}{{c |}}{{{b}}}", buffer_sizes, "") + """\\\\
		 	\\hline
		 	\\hline
""" 
	
	for file_size in table_data:
		row = f"\\textbf{{{file_size}}} & "
		for buffer_size in table_data[file_size]:
			list_of_vals = table_data[file_size][buffer_size]
			avg = sum(list_of_vals) / len(list_of_vals)

			row += "%.2f & " % avg

		for _ in range(len(buffer_sizes) - len(table_data[file_size])):
			row += "{} & "

		table += row[:-2] + "\\\\\n"

	table += """

			\\hline

		\\end{tabular}}
		\\label{tbl:data_""" + underscore_case(fs + " " + report.name) + """}
	\\end{center}
\\end{table}
	"""

	return table


def generate_tables(result: IOZoneResult, out_dir: str):
	for r in result:
		table = generate_table(r, result.fs, result.identifier)
		filename = f"{r.name}-{result.identifier}-table.tex"
		with open(f"{out_dir}/{filename}", "w+") as f:
			f.write(table)
		print(f"Generated table for {filename}")