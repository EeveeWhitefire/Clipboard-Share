import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ("79.177.199.153", 1337)
sock.connect(server_address)
message = "{}\x00\x00{}".format(input("enter type:"),input("enter text: ")).encode("utf-8")
sock.send(message)
input("enter any key to exit")
sock.close()