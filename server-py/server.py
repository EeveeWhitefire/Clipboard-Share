import socket
import sys
import atexit
import traceback
from _thread import start_new_thread as thread

from c_scripts.get_local_ipv4 import main as ipv4

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

# Bind the socket to the port

PORT = 42069
IP = ipv4() #my ipv4
MAX_BUFF_SIZE = 2048
MAX_CONN = 10

connections = {}

def cleanup():
    for addr, conn in list(connections.items()):
        conn.close()
        if addr in connections:
            connections.pop(addr)
    
    sock.close()

def listen(client_address):
    i = 1
    try:
        while i:
            content = connections[client_address].recv(MAX_BUFF_SIZE)
            content = content.strip().strip(b"\x00")
            if content == b'':
                continue

            cf_type = "UNICODE"
            
            for addr in connections:
                if addr != client_address:
                    try:
                        connections[addr].send(content)
                    except:
                        connections.pop(addr)
            
            content = content.decode()
            print("{}: New message : Type: {}, Content: {}".format(client_address, cf_type, content))
    except ConnectionResetError:
        pass
    except:
        pass
        # print("{}: {}".format(client_address, traceback.format_exc()))
    finally:
        i = 0
        # Clean up the connection
        if client_address in connections:
            connections[client_address].close()
            connections.pop(client_address)
        print("{}: Disconnected".format(client_address))
    
    return

def main():
    server_address = (IP, PORT)
    print('Starting up on {} port {}'.format(*server_address))
    sock.bind(server_address)

    # Listen for incoming connections
    sock.listen(MAX_CONN)
    atexit.register(cleanup)

    # Wait for a connection
    print('Listening for new connections\n')
    
    try:
        while True:
            if len(connections) >= MAX_CONN:
                break
            
            connection, client_address = sock.accept()
            connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            connections[client_address] = connection
            
            print('{}: Connected'.format(client_address))
            thread(listen, (client_address,))
    except Exception as e:
        print("{}: {}".format(client_address, traceback.format_exc()))
    except:
        pass
    finally:
        print("Closing connections")

    return
    
main()
sock.close()
