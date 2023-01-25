import diagramgen.src.parser as parser
import diagramgen.src.diagram as diagram

def run():
	reports = parser.parse_files("saved.nosync", "FFS-iozone")

	write_data = []
	for report in reports:
		write_report = report["Write"]

		print(report.identifier)
		write_data.extend(write_report.data[0].raw())

	diagram.create_bell(write_data)