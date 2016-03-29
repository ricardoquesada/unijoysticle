#!/usr/bin/python
# ----------------------------------------------------------------------------
# Test for c64-remote-control
# ----------------------------------------------------------------------------
'''
Test for c64-remote-control
'''

from __future__ import division, unicode_literals, print_function

import socket
import time
import sys
import os

if len(sys.argv) is not 2:
    print("%s v0.1 - A tool to test the c64-remote-control\n" % os.path.basename(sys.argv[0]))
    print("\nUsage: %s 0|1" % os.path.basename(sys.argv[0]))
    print("Example:\n%s 0" % os.path.basename(sys.argv[0]))
    print("%s 1" % os.path.basename(sys.argv[0]))
    sys.exit(-1)

joyport = int(sys.argv[1])
UDP_IP = "10.0.0.24"
UDP_PORT = 6464

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %d" % UDP_PORT)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

values = [1,2,4,8,16,8,4,2,
          1,2,4,8,16,8,4,2,
          1,2,4,8,16,8,4,2,
          1,2,4,8,16,8,4,2,
          1,2,4,8,16,8,4,2,
          1,2,4,8,16,8,4,2,
          31,31,31,31,0,0,0,0,
          31,31,31,31,0,0,0,0,
          31,31,31,31,0,0,0,0,
          31-1,31-2,31-4,31-8,31-16,31-8,31-4,31-2,
          31-1,31-2,31-4,31-8,31-16,31-8,31-4,31-2,
          31-1,31-2,31-4,31-8,31-16,31-8,31-4,31-2,
          31-1,31-2,31-4,31-8,31-16,31-8,31-4,31-2,
          31-1,31-2,31-4,31-8,31-16,31-8,31-4,31-2,
          31-1,31-2,31-4,31-8,31-16,31-8,31-4,31-2,
          31,31,31,31,0,0,0,0,
          31,31,31,31,0,0,0,0,
          31,31,31,31,0,0,0,0]

index = 0

while True:
    sock.sendto(chr(joyport) + chr(values[index]) , (UDP_IP, UDP_PORT))
    time.sleep(0.05)
    index = index + 1
    index = index % len(values)
