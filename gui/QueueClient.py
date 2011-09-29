#!/usr/bin/env python

import os
import socket
import sys

# Configuration
HOST, PORT = "localhost", 7777

if len(sys.argv) != 4:
    print >>sys.stderr, "Usage: QueueClient.py model.cellml proto.xml output_folder"
    sys.exit(1)

sock = socket.socket()
sock.connect((HOST, PORT))
msg = '\n'.join(sys.argv[1:] + [''])
print "Sending:\n", msg
sock.send(msg)
print "Received:"
print sock.recv(2**10)
sock.close()
