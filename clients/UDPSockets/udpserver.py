#!/usr/bin/env python

import sys, os
import socket

"""
Main function
"""
def main():    
    UDP_IP = '127.0.0.1'
    UDP_PORT = 1234
    BUFFER_SIZE = 1024
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    
    while True:
        data, addr = sock.recvfrom(BUFFER_SIZE)
        print "received data:", data
    
if __name__ == '__main__':
  main()