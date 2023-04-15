import unittest
import unittest.mock as mock

from benchmark.src.measure import get_log_path

class BenchmarkTest(unittest.TestCase):

	def test_get_path_ok_for_existing_path(self):

		def new_exists():
			def func(path, **kwargs):
				return path == "logs/ffs-iter-1"
			return func

		with mock.patch("os.path.exists", new_callable=new_exists):
			path = get_log_path("ffs", 1)
			self.assertEqual(path, "logs/ffs-iter-1(new).log")