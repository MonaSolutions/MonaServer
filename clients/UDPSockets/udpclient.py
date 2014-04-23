#!/usr/bin/env python

import sys, os
import socket

"""
Main function
"""
def main():
    message = ""
    if len(sys.argv) > 1:
        message = sys.argv[1]      
    else:
        (filepath, filename) = os.path.split(sys.argv[0])
        print "Usage: \n\n" + filename + " <message>"
        return -1
    
    UDP_IP = '127.0.0.1'
    UDP_PORT = 1235
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # SOCK_DGRAM means UDP
    sock.sendto(message, (UDP_IP, UDP_PORT))
    
if __name__ == '__main__':
  main()