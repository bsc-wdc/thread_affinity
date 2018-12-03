import unittest
import random
import subprocess
import signal
import sys
import os
import thread_affinity

def get_random_mask():
	"""Return a random, valid affinity mask
	Which is a subset of {0, 1, ..., 2 ** num_procs - 1}
	"""
	num_procs = thread_affinity.get_nprocs()
	r = random.randint(1, 2 ** num_procs)
	return [i for i in range(num_procs) if (r & (1 << i))]

class TestThreadAffinityLibrary(unittest.TestCase):
	"""Test basic Thread Affinity features.
	"""
	def test_set_get_affinity(self):
		"""Test if a simple set & get works
		"""
		# Test results may vary if executed in different systems
		# with different amount of CPUUs
		random.seed(1)
		proc_list = get_random_mask()
		thread_affinity.setaffinity(proc_list)
		self.assertEqual(proc_list, thread_affinity.get_affinity())

	def test_set_get_incorrect_affinity(self):
		"""Test if the program sets the default affinity in case of illegal masks
		"""
		illegal_mask = [-1]
		default_affinity = thread_affinity.get_default_affinity()
		thread_affinity.setaffinity(illegal_mask)
		self.assertEqual(default_affinity, thread_affinity.get_affinity())

	def test_set_get_affinity_subprocess(self):
		"""Test if the affinity of a subprocess can be controlled from above
		"""
		# Test results may vary if executed in different systems
		# with different amount of CPUUs
		random.seed(3)
		proc_list = get_random_mask()
		import subprocess
		proc = subprocess.Popen(["python", "-c", "while True: pass"])
		thread_affinity.set_affinity(proc_list, proc.pid)
		self.assertEqual(proc_list, thread_affinity.get_affinity(proc.pid))
		proc.send_signal(signal.SIGKILL)


if __name__ == '__main__':
	suite = unittest.TestLoader().loadTestsFromTestCase(TestThreadAffinityLibrary)
	unittest.TextTestRunner(verbosity = 2).run(suite)
