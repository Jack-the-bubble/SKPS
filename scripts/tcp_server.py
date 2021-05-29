import socket
import sys
from _thread import start_new_thread
from scripts import gesture


HOST = '' # all availabe interfaces
PORT = 9999 # arbitrary non privileged port

SIZE = 1228800
FILE_NAME = 'in-img.ppm'
chunk_size = 1024
chunk_num = SIZE/chunk_size
led_feedback = None
max_img_num = 30
# self.request - TCP socket connected to the client
chunk_idx = 0
different_chunks = 0
in_file = open(FILE_NAME, 'w')
in_file.write('P6\n1280 960 255\n')
in_file.close()
in_file = open(FILE_NAME, 'ab')

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

# def client_thread(conn):
#     conn.send("Welcome to the Server. Type messages and press enter to send.\n")
#
#     while True:
#         data = conn.recv(1024)
#         if not data:
#             break
#         reply = "OK . . " + data
#         conn.sendall(reply)
#     conn.close()

while True:
    # blocking call, waits to accept a connection

    # connect client with led
    led_conn, led_addr = s.accept()
    print("[-] Connected to " + led_addr[0] + ":" + str(led_addr[1]))

    # connect client streaming images
    img_conn, img_addr = s.accept()
    print("[-] Connected to " + led_addr[0] + ":" + str(led_addr[1]))
    try:
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
                #########
                gest = gesture.main()
                print("Gest is {}".format(gest))
                #########
                if gest == 1:
                    led_conn.sendall("on ".encode())
                else:
                    led_conn.sendall("off".encode())

                led_feedback = led_conn.recv(2)

                in_file = open(FILE_NAME, 'w')
                in_file.write('P6\n1280 960 255\n')
                in_file.close()
                in_file = open(FILE_NAME, 'ab')

    except OSError as e:
        print(e)
        s.close()
