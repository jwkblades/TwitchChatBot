#!/usr/bin/python
import socket
import time

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
client.connect(("localhost", 60000));
print "Message: ";
msg = raw_input();
sent = client.send(msg);
print "Sent: {0:d} bytes\n".format(sent);
time.sleep(1);
resp = client.recv(1024);
print "Received response: '{0:s}'\n".format(resp);
client.close();
