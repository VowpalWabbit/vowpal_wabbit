import sys
import unittest
import socket
from threading import Thread
import json
import ssl

from http.server import BaseHTTPRequestHandler, HTTPServer

# add library output path
sys.path.append('swig')

from decision_service import *

def get_free_port():
	s = socket.socket(socket.AF_INET, type=socket.SOCK_STREAM)
	s.bind(('localhost', 0))
	address, port = s.getsockname()
	s.close()
	return port

class MockHandler(BaseHTTPRequestHandler):

	def __init__(self, mock_server, *args):
		self.mock_server = mock_server
		BaseHTTPRequestHandler.__init__(self, *args)

	def do_GET_config(self):
		# Process an HTTP GET request and return a response with an HTTP 200 status.
		self.send_response(200)
		self.send_header('Content-Type', 'application/json; charset=utf-8')
		self.end_headers()

		response_content = json.dumps({"ModelBlobUri": "http://localhost:%d/model" % self.mock_server.mock_server_port,
			"EventHubInteractionConnectionString": "conn1",
			"EventHubObservationConnectionString": "conn2"
		})
		self.wfile.write(response_content.encode('utf-8'))

		# TODO: expose additional properties: https/http (or enable simpelhttp https)
		# expose cert validation

		self.mock_server.get = self.mock_server.get + 1

	def do_GET(self):
		if self.path == "/config.json":
			return self.do_GET_config()

		self.send_response(404)
		self.end_headers()

	# disable logging to stdout
	def log_message(self, format, *args):
		return

class MockServer:
	def __init__(self, https_enabled = True):
		self.https_enabled = https_enabled

	def __enter__(self):
		self.get = 0;
		# Configure mock server.
		self.mock_server_port = get_free_port()

		def handler(*args):
			MockHandler(self, *args)

		self.mock_server = HTTPServer(('localhost', self.mock_server_port), handler)
		proto = 'http'
		if self.https_enabled:
			self.mock_server.socket = ssl.wrap_socket (self.mock_server.socket, server_side=True,
									certfile='unittest.pem')
			proto = 'https'

		# Start running mock server in a separate thread.
		# Daemon threads automatically shut down when the main process exits.
		self.mock_server_thread = Thread(target=self.mock_server.serve_forever)
		self.mock_server_thread.setDaemon(True)
		self.mock_server_thread.start()

		self.base_url = '{proto}://localhost:{port}/'.format(proto=proto, port=self.mock_server_port)

		return self

	def __exit__(self, exception_type, exception_value, trackback):
		self.mock_server.server_close()

class TestDecisionServiceConfiguration(unittest.TestCase):

	def test_download(self):
		# can't easily disable cert validation w/o introducing yet another method (optional params not supported)
		with MockServer(https_enabled = False) as server:
			config = DecisionServiceConfiguration_Download(server.base_url + 'config.json')

			self.assertEqual(config.model_url, server.base_url + 'model')

			# make sure it's called once'
			self.assertEqual(server.get, 1, 'HTTP server not called')

	def test_not_found(self):
		with MockServer() as server:
			with self.assertRaises(SystemError):
				DecisionServiceConfiguration_Download(server.base_url + 'notfound')

	def test_upload(self):
		self.assertEqual(1,1)

if __name__ == '__main__':
	unittest.main()

# config = DecisionServiceConfiguration_Download("https://storagecloezroez3lrg.blob.core.windows.net/mwt-settings/client?sv=2017-04-17&sr=b&sig=j86B1Ir1z9UC8KJP9QJ7R62ESecFBCPr9BH6tx1AaL4%3D&st=2018-01-04T15%3A06%3A38Z&se=2028-01-04T15%3A07%3A38Z&sp=r")
#print("hello world %s" % config.model_url)
# client = DecisionServiceClient(config)
# client.rank('{"a":123}', "abc", [1, 4, 5]);
