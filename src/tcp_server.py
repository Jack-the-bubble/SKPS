import socket
import sys
from _thread import start_new_thread


HOST = '' # all availabe interfaces
PORT = 9999 # arbitrary non privileged port

SIZE = 1228800
chunk_size = 1024
chunk_num = SIZE/chunk_size
file_num = 0
led_feedback = None
# self.request - TCP socket connected to the client
chunk_idx = 0
different_chunks = 0
in_file = open('in_img-{}.ppm'.format(file_num), 'w')
in_file.write('P6\n1280 960 255\n')
in_file.close()
in_file = open('in_img-{}.ppm'.format(file_num), 'ab')

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error as msg:
    print("Could not create socket. Error Code: ", str(msg[0]), "Error: ", msg[1])
    sys.exit(0)

print("[-] Socket Created")

# bind socket
try:
    s.bind((HOST, PORT))
    print("[-] Socket Bound to port " + str(PORT))
except socket.error as msg:
    print("Bind Failed. Error Code: {} Error: {}".format(str(msg[0]), msg[1]))
    sys.exit()

s.listen(10)
print("Listening...")

# The code below is what you're looking for ############

def client_thread(conn):
    conn.send("Welcome to the Server. Type messages and press enter to send.\n")

    while True:
        data = conn.recv(1024)
        if not data:
            break
        reply = "OK . . " + data
        conn.sendall(reply)
    conn.close()

while True:
    # blocking call, waits to accept a connection


    led_conn, led_addr = s.accept()
    print("[-] Connected to " + led_addr[0] + ":" + str(led_addr[1]))


    img_conn, img_addr = s.accept()
    print("[-] Connected to " + led_addr[0] + ":" + str(led_addr[1]))

    while True:
        if chunk_idx < chunk_num:
            data = img_conn.recv(chunk_size).strip()

            empty_bytes = chunk_size - len(data)
            in_file.write(data)
            for b in range(empty_bytes):
                in_file.write(b' ')
                different_chunks = different_chunks + 1

            chunk_idx = chunk_idx + 1
            img_conn.sendall("ok".encode())

        else:
            in_file.close()
            chunk_idx = 0
            # send data to led client
            if file_num % 2 == 0:
                led_conn.sendall("on ".encode())
            else:
                led_conn.sendall("off".encode())

            led_feedback = led_conn.recv(2)

            file_num = file_num + 1
            in_file = open('in_img-{}.ppm'.format(file_num), 'w')
            in_file.write('P6\n1280 960 255\n')
            in_file.close()
            in_file = open('in_img-{}.ppm'.format(file_num), 'ab')


    # start_new_thread(client_thread, (conn,))

s.close()