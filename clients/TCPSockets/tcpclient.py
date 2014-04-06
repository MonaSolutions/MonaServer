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
    
    TCP_IP = '127.0.0.1'
    TCP_PORT = 1235
    BUFFER_SIZE = 1024
    
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # SOCK_STREAM means TCP
    s.connect((TCP_IP, TCP_PORT))
    s.send(message)
    data = s.recv(BUFFER_SIZE)
    s.close()
    
    print "received data:", data
    
if __name__ == '__main__':
  main()