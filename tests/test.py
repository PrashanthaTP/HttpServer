import unittest
import requests

url="http://localhost:8080"
n_repeat_count = 100
class TestHttpServer(unittest.TestCase):
	def test_get_method_with_valid_path(self):
		for _ in range(n_repeat_count):
			response = requests.get(url)
			self.assertEqual(response.text,"C++ is Cool\n")
			self.assertEqual(response.status_code,200)

	def test_get_method_with_invalid_path(self):
		for _ in range(n_repeat_count):
			response = requests.get(f"{url}/path.html")
			self.assertEqual(response.text,"Resource not found")
			self.assertEqual(response.status_code,404)

	def test_post_method(self):
		for _ in range(n_repeat_count):
			response = requests.post(url, data={})
			self.assertEqual(response.text,"Http Method ( POST ) Not Supported")
			self.assertEqual(response.status_code,405)

if __name__ == "__main__":
	unittest.main()
