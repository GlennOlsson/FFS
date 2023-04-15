import unittest
import unittest.mock as mock

from src.sniff_parser import parse_box, parse_boxes, parse

box_1 = """
=======================================
| IO Statistics                       |
|                                     |
| Duration: 223.262709 secs           |
| Interval:  10 secs                  |
|                                     |
| Col 1: Frames and bytes             |
|-------------------------------------|
|            |1                       |
| Interval   |  Frames  |    Bytes    |
|-------------------------------------|
|   0 <>  10 |     1875 |     1299494 |
|  10 <>  20 |     3010 |     2399567 |
|  20 <>  30 |      738 |      201989 |
|  30 <>  40 |     3114 |     2434954 |
|  40 <>  50 |     1840 |     1303932 |
|  50 <>  60 |     3016 |     2417996 |
|  60 <>  70 |     1953 |     1323301 |
|  70 <>  80 |     3068 |     2435268 |
|  80 <>  90 |     1783 |     1281329 |
|  90 <> 100 |     1882 |     1309415 |
| 100 <> 110 |     3174 |     2443134 |
| 110 <> 120 |      427 |      119911 |
| 120 <> 130 |     4734 |     3701505 |
| 130 <> 140 |      702 |      196025 |
| 140 <> 150 |     3078 |     2405492 |
| 150 <> 160 |     1983 |     1341734 |
| 160 <> 170 |     3101 |     2421074 |
| 170 <> 180 |     1867 |     1297048 |
| 180 <> 190 |     3179 |     2444297 |
| 190 <> 200 |     1839 |     1296108 |
| 200 <> 210 |     3141 |     2431147 |
| 210 <> 220 |     5633 |     4681867 |
| 220 <> Dur |      610 |      172314 |
=======================================
"""

box_2 = """
=======================================
| IO Statistics                       |
|                                     |
| Duration: 251.188204 secs           |
| Interval:  10 secs                  |
|                                     |
| Col 1: Frames and bytes             |
|-------------------------------------|
|            |1                       |
| Interval   |  Frames  |    Bytes    |
|-------------------------------------|
|   0 <>  10 |     1932 |     1304519 |
|  10 <>  20 |     3107 |     2406404 |
|  20 <>  30 |      640 |      176829 |
|  30 <>  40 |     3185 |     2433311 |
|  40 <>  50 |     2031 |     1337675 |
|  50 <>  60 |     1824 |     1274773 |
|  60 <>  70 |     3129 |     2412893 |
|  70 <>  80 |     1906 |     1199559 |
|  80 <>  90 |     3353 |     2582129 |
|  90 <> 100 |     2045 |     1306887 |
| 100 <> 110 |     3242 |     2471976 |
| 110 <> 120 |      626 |      175565 |
| 120 <> 130 |     4540 |     3577415 |
| 130 <> 140 |      612 |      172240 |
| 140 <> 150 |     3187 |     2429193 |
| 150 <> 160 |      142 |       49680 |
| 160 <> 170 |     2122 |     1400260 |
| 170 <> 180 |     3391 |     2492592 |
| 180 <> 190 |     1915 |     1301789 |
| 190 <> 200 |     3347 |     2479001 |
| 200 <> 210 |     1977 |     1326112 |
| 210 <> 220 |     4295 |     3001075 |
| 220 <> 230 |     5730 |     4508554 |
| 230 <> 240 |     2082 |     1433608 |
| 240 <> 250 |     6371 |     5193427 |
| 250 <> Dur |     1147 |      658806 |
=======================================
"""

double_boxes = f"""
{box_1}

{box_2}
"""

class TestSniffParser(unittest.TestCase):

	def test_parses_correctly(self):
		result = parse_box(box_1)

		first = result[0]
		self.assertEqual(first.frames, 1875)
		self.assertEqual(first.bytes, 1299494)
		self.assertEqual(first.duration, 10)

		mid = result[11]
		self.assertEqual(mid.frames, 427)
		self.assertEqual(mid.bytes, 119911)
		self.assertEqual(mid.duration, 10)


		last = result[22]
		self.assertEqual(last.frames, 610)
		self.assertEqual(last.bytes, 172314)
		self.assertAlmostEqual(last.duration, 3.263)

	def test_parses_multiple_correctly(self):
		result = parse_boxes(double_boxes)

		sniff1 = result[0]
		first = sniff1[0]
		self.assertEqual(first.frames, 1875)
		self.assertEqual(first.bytes, 1299494)
		self.assertEqual(first.duration, 10)

		mid = sniff1[11]
		self.assertEqual(mid.frames, 427)
		self.assertEqual(mid.bytes, 119911)
		self.assertEqual(mid.duration, 10)


		last = sniff1[22]
		self.assertEqual(last.frames, 610)
		self.assertEqual(last.bytes, 172314)
		self.assertAlmostEqual(last.duration, 3.263)



		sniff2 = result[1]
		first = sniff2[0]
		self.assertEqual(first.frames, 1932)
		self.assertEqual(first.bytes, 1304519)
		self.assertEqual(first.duration, 10)

		mid = sniff2[11]
		self.assertEqual(mid.frames, 626)
		self.assertEqual(mid.bytes, 175565)
		self.assertEqual(mid.duration, 10)


		last = sniff2[25]
		self.assertEqual(last.frames, 1147)
		self.assertEqual(last.bytes, 658806)
		self.assertAlmostEqual(last.duration, 1.188)
	
	@mock.patch("src.sniff_parser.read_file")
	def test_parse_returns_correct_sniff(self, mock_func: mock.Mock):
		mock_func.return_value = double_boxes

		s = parse("some-path")

		self.assertEqual(len(s.entries), 49)

		first = s[0]
		self.assertEqual(first.frames, 1875)
		self.assertEqual(first.bytes, 1299494)
		self.assertEqual(first.duration, 10)

		mid = s[11]
		self.assertEqual(mid.frames, 427)
		self.assertEqual(mid.bytes, 119911)
		self.assertEqual(mid.duration, 10)


		last = s[22]
		self.assertEqual(last.frames, 610)
		self.assertEqual(last.bytes, 172314)
		self.assertAlmostEqual(last.duration, 3.263)



		first = s[23]
		self.assertEqual(first.frames, 1932)
		self.assertEqual(first.bytes, 1304519)
		self.assertEqual(first.duration, 10)

		mid = s[23 + 11]
		self.assertEqual(mid.frames, 626)
		self.assertEqual(mid.bytes, 175565)
		self.assertEqual(mid.duration, 10)


		last = s[23 + 25]
		self.assertEqual(last.frames, 1147)
		self.assertEqual(last.bytes, 658806)
		self.assertAlmostEqual(last.duration, 1.188)