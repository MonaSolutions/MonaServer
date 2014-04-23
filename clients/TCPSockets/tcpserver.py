#!/usr/bin/env python

import socket

TCP_IP = '127.0.0.1'
TCP_PORT = 1234

BUFFER_SIZE = 1024

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((TCP_IP, TCP_PORT))
s.listen(5)

while 1:
    conn, addr = s.accept()
    print 'New Connection from:', addr
    while 1:
        data = conn.recv(BUFFER_SIZE)

        if not data: break
        
        print "received data:", data
        conn.sendall(data)  # echo

conn.close()