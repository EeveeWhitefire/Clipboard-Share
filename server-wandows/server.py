import socket
import sys
import atexit
from _thread import start_new_thread as thread

from c_scripts.get_local_ipv4 import main as ipv4

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

# Bind the socket to the port

PORT = 1337
IP = ipv4() #my ipv4

CF_TEXT = 1
CF_UNICODETEXT = 13
CF_BITMAP = 2
CF_DIB = 8
CF_DIBV5 = 17

MAX_BUFF_SIZE = 2048
MAX_CONN = 10
DELIM = b"\x00\x00"

connections = {}

def cleanup():
    for addr, conn in list(connections.items()):
        conn.close()
        connections.pop(addr)
    
    sock.close()

def listen(client_address):
    i = 1
    
    try:
        connections[client_address].send(b"noot")
        while i:
            message = connections[client_address].recv(MAX_BUFF_SIZE)
            delim_pos = message.find(DELIM)
            if message == b'' or delim_pos == -1:
                continue
            
            cf_type = int(message[:delim_pos])
            content = message[delim_pos + 2:]
            message = message[:delim_pos + 2]
            
            if cf_type == CF_UNICODETEXT or cf_type == CF_TEXT:
                content = content.decode("utf-8")
                
            if cf_type == CF_TEXT:
                message = message + content.encode("ansi")
            elif cf_type == CF_UNICODETEXT:
                message = message + content.encode("utf-16le")
            
            for addr in connections:
                if addr != client_address:
                    connections[addr].send(message)
              
            print("From : {}, Type: {}, Content: {}\n".format(client_address, cf_type, content))
    except ConnectionResetError:
        pass
    except Exception as e:
        print("{}:{}".format(client_address, e))
    finally:
        i = 0
        # Clean up the connection
        connections[client_address].close()
        connections.pop(client_address)
        print("lost connection with {}\n".format(client_address))
    
    return

def main():
    server_address = (IP, PORT)
    print('starting up on {} port {}'.format(*server_address))
    sock.bind(server_address)
    # Listen for incoming connections
    sock.listen(MAX_CONN)
    atexit.register(cleanup)
    
    try:
        while True:
            if len(connections) >= MAX_CONN:
                break
            # Wait for a connection
            print('waiting for a connection')
            
            connection, client_address = sock.accept()
            connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            connections[client_address] = connection
            
            print('connection from {}\n'.format(client_address))
            thread(listen, (client_address,))
    except Exception as e:
        print("{}:{}".format(client_address, e))
    finally:
        connection.close()
        sock.close()
    
main()