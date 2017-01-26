#!/usr/bin/python
from __future__ import division, unicode_literals, print_function
import socket
import sys
import struct
import os
import time

UDP_PORT = 6464

def send_packet(ipaddress, port, joyvalue, potx=0, poty=0):

    MESSAGE = "Hello, World!"

    print("target IP/Port %s/%d" % (ipaddress, UDP_PORT))

    joyvalue = int(joyvalue)
    port = int(port)
    potx = int(potx)
    poty = int(poty)

    print("Sending to control port:%d  joy=%d, potx=%d, poty=%d" % (port, joyvalue, potx, poty))

    message = struct.pack("BBBBBBBB",3,0,0,0,potx,poty,0,0)
    sock = socket.socket(socket.AF_INET, # Internet
                         socket.SOCK_DGRAM) # UDP
    sock.sendto(message, (ipaddress, UDP_PORT))

def help():
    print("%s v0.1 - A tool to test the UniJoystiCle\n" % os.path.basename(sys.argv[0]))
    print("%s ip_address port joy_value potx_value poty_value")
    print("Example:\n%s 192.168.4.1 0 255 0 0" % os.path.basename(sys.argv[0]))
    sys.exit(-1)


if __name__ == "__main__":

    if len(sys.argv) == 2 and sys.argv[1] == "test":
        while True:
            for i in range(250):
                send_packet('10.0.0.15', 0, 0, i, 0)
                time.sleep(0.05)
            time.sleep(2)
            for i in range(250):
                send_packet('10.0.0.15', 0, 0, 250-i, 0)
                time.sleep(0.05)
            time.sleep(2)

    if len(sys.argv) <= 4:
        help()

    args = sys.argv[1:]

    for i in range(1,2):
        send_packet(*args)


