import socket
import sys

from c_scripts.get_local_ipv4 import main as ipv4
# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

PORT = 42069
IP = ipv4() #my ipv4
# Connect the socket to the port where the server is listening
server_address = (IP, PORT)
sock.connect(server_address)
message = "{}".format(input("enter text: ")).encode("utf-8")
sock.send(message)
sock.close()
# input("enter any key to exit")
