#!/usr/bin/python
import socket
import sys
import time

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
client.connect(("localhost", 60000));
m = " ".join(sys.argv[1:]);
sent = client.send(m);
print "Sent: {0:d} bytes '{1:s}'\n".format(sent, m);
time.sleep(.1);
client.close();
