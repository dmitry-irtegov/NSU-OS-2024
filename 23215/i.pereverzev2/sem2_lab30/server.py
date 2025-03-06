import socket
import time

HOST = '127.0.0.1'
PORT = 8881
RESPONSE_HEADER = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n"

def handle_client(conn, addr):
    print("Connected by", addr)
    try:
        request = conn.recv(1024).decode('ascii')

        if request.startswith('GET'):
            conn.sendall(RESPONSE_HEADER.encode('ascii'))
            for i in range(25):
                conn.sendall("String number {} 1".format(i + 26).encode('ascii'))
                conn.sendall("String number {} 2 ".format(i + 26).encode('ascii'))
                conn.sendall(" String number {} 3\n".format(i + 26).encode('ascii'))


            for i in range(50):
                time.sleep(1)
                conn.sendall("String number {} 1".format(i + 26).encode('ascii'))
                conn.sendall(" String number {} 2\n".format(i + 26).encode('ascii'))
    except Exception as e:
        print('error handling client')
    finally:
        conn.close()


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

s.bind((HOST, PORT))
s.listen(1)

try:
    while True:
        conn, addr = s.accept()
        handle_client(conn, addr)

except KeyboardInterrupt:
    print("\nShutting down server...")
finally:
    s.close()