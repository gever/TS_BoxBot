import time
import sys
import http.server
from urllib.parse import urlparse, parse_qs
import json

MOVE_PER_SECOND = 10
TURN_PER_SECOND = 10

movement_finish_time = 0

class SimbotHandler(http.server.BaseHTTPRequestHandler):
  def bad_request(self, msg):
    self.send_response(400)
    self.end_headers()
    self.wfile.write(b'400 Bad Request: ' + msg.encode('utf-8'))

  def reply_json(self, obj):
    jstr = json.dumps(obj)

    self.send_response(200)
    self.send_header('Content-Type', 'application/json')
    self.end_headers()
    self.wfile.write(jstr.encode('utf-8'))

  # set CORS headers so client can access via localhost
  # def do_OPTIONS(self):
  #   self.send_response(200)
  #   self.send_header('Access-Control-Allow-Origin', '*')

  def end_headers (self):
    self.send_header('Access-Control-Allow-Origin', '*')
    http.server.BaseHTTPRequestHandler.end_headers(self)

  def do_GET(self):
    global movement_finish_time

    purl = urlparse(self.path)
    path = purl.path
    params = parse_qs(purl.query)

    if path == '/move':
      if 'dist' not in params:
        self.bad_request('missing dist parameter')
        return

      try:
        dist = float(params['dist'][0])
      except ValueError:
        self.bad_request('invalid dist parameter')
        return

      print('move', dist)

      movement_finish_time = time.time() + abs(dist)/MOVE_PER_SECOND

      self.reply_json({'status': 'ACK'})
    elif path == '/turn':
      if 'angle' not in params:
        self.bad_request('missing angle parameter')
        return

      try:
        angle = float(params['angle'][0])
      except ValueError:
        self.bad_request('invalid angle parameter')
        return

      print('turn', angle)

      movement_finish_time = time.time() + abs(angle)/TURN_PER_SECOND

      self.reply_json({'status': 'ACK'})
    elif path == '/stop':
      print('stop')
      movement_finish_time = 0
      self.reply_json({'status': 'ACK'})
    elif path == '/busy':
      busy = time.time() < movement_finish_time
      print('busy', busy)
      self.reply_json({'busy': busy})
    else:
      self.send_response(404)
      self.end_headers()
      self.wfile.write(b'404 Not Found')

port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
server = http.server.HTTPServer(('localhost', port), SimbotHandler)
print('Starting simbot server on port', port, '...')
server.serve_forever()
