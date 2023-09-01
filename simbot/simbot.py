import os
import time
import sys
import http.server
from urllib.parse import urlparse, parse_qs
import json
import random

MOVE_PER_SECOND = 10
TURN_PER_SECOND = 10
SERVO_TURN_PER_SECOND = 600

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
    elif path == '/servoInit':
      if 'pin' not in params:
        self.bad_request('missing pin parameter')
        return

      try:
        pin = int(params['pin'][0])
      except ValueError:
        self.bad_request('invalid pin parameter')
        return

      print('servoInit', pin)

      self.reply_json({'status': 'ACK'})
    elif path == '/servoGo':
      if 'pin' not in params:
        self.bad_request('missing pin parameter')
        return

      if 'angle' not in params:
        self.bad_request('missing angle parameter')
        return

      try:
        pin = int(params['pin'][0])
      except ValueError:
        self.bad_request('invalid pin parameter')
        return

      try:
        angle = int(params['angle'][0])
      except ValueError:
        self.bad_request('invalid angle parameter')
        return

      if abs(angle) > 90:
        self.bad_request('angle must be between -90 and 90')
        return

      print('servoGo', pin, angle)

      movement_finish_time = time.time() + abs(angle)/SERVO_TURN_PER_SECOND

      self.reply_json({'status': 'ACK'})
    elif path == '/led':
      if 'pin' not in params:
        self.bad_request('missing pin parameter')
        return

      if 'state' not in params:
        self.bad_request('missing state parameter')
        return

      try:
        pin = int(params['pin'][0])
      except ValueError:
        self.bad_request('invalid pin parameter')
        return

      try:
        state = int(params['state'][0])
      except ValueError:
        self.bad_request('invalid state parameter')
        return

      if (state != 0) and (state != 1):
        self.bad_request('state must be 0 or 1')
        return

      print('led', pin, state)

      self.reply_json({'status': 'ACK'})
    elif path == '/luminosity1':
      val = random.randint(0, 1000)
      print('luminosity1', val)
      self.reply_json({'luminosity1': val})
    elif path == '/luminosity2':
      val = random.randint(0, 1000)
      print('luminosity2', val)
      self.reply_json({'luminosity2': val})
    elif path == '/distance':
      val = random.randint(0, 100)
      print('distance', val)
      self.reply_json({'distance': val})
    elif path == '/stop':
      print('stop')
      movement_finish_time = 0
      self.reply_json({'status': 'ACK'})
    elif path == '/busy':
      busy = time.time() < movement_finish_time
      print('busy', busy)
      self.reply_json({'busy': busy})
    else:
      # serve static files
      print('request for path:', path)

      if path == '/':
        path = '/blox.html'

      contentType = "text/plain"
      if (path.endswith(".html")):
        contentType = "text/html"
      elif (path.endswith(".js")):
        contentType = "text/javascript"
      elif (path.endswith(".css")):
        contentType = "text/css"
      elif (path.endswith(".ico")):
        contentType = "image/x-icon"
      elif (path.endswith(".mp3")):
        contentType = "audio/mpeg"
      elif (path.endswith(".png")):
        contentType = "image/png"
      elif (path.endswith(".svg")):
        contentType = "image/svg+xml"
      elif (path.endswith(".jpg")):
        contentType = "image/jpeg"
      elif (path.endswith(".zip") or path.endswith(".gz")):
        contentType = "application/javascript"

      assert path.startswith('/')
      fn = os.path.join('../data', path[1:])
      print('serving file', repr(fn), 'with content type', contentType)

      try:
        with open(fn, 'rb') as f:
          self.send_response(200)
          self.send_header('Content-Type', contentType)
          if (path.endswith(".zip") or path.endswith(".gz")):
            self.send_header('Content-Encoding', 'gzip')
          self.end_headers()
          self.wfile.write(f.read())
      except FileNotFoundError:
        print('file not found')
        self.send_response(404)
        self.end_headers()
        self.wfile.write(b'404 Not Found')

port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
server = http.server.HTTPServer(('0.0.0.0', port), SimbotHandler)
print('Starting simbot server on port', port, '...')
server.serve_forever()
