#!/usr/bin/python3

import socketserver

SIZE = 1228800

class Handler_TCPServer(socketserver.BaseRequestHandler):
    """
    The TCP Server class for demonstration.

    Note: We need to implement the Handle method to exchange data
    with TCP client.

    """

    def handle(self):
        chunk_size = 1024
        chunk_num = SIZE/chunk_size
        file_num = 0
        # self.request - TCP socket connected to the client
        chunk_idx = 0
        different_chunks = 0
        in_file = open('in_img-{}.ppm'.format(file_num), 'w')
        in_file.write('P6\n1280 960 255\n')
        in_file.close()
        in_file = open('in_img-{}.ppm'.format(file_num), 'ab')
        while True:
            if chunk_idx < chunk_num:
                self.data = self.request.recv(chunk_size).strip()

                empty_bytes = chunk_size - len(self.data)
                in_file.write(self.data)
                for b in range(empty_bytes):
                    in_file.write(b' ')
                    different_chunks = different_chunks + 1

                chunk_idx = chunk_idx + 1
                self.request.sendall("ok".encode())

            else:
                in_file.close()
                chunk_idx = 0
                file_num = file_num + 1
                in_file = open('in_img-{}.ppm'.format(file_num), 'w')
                in_file.write('P6\n1280 960 255\n')
                in_file.close()
                in_file = open('in_img-{}.ppm'.format(file_num), 'ab')



if __name__ == "__main__":
    # HOST, PORT = "localhost", 9999
    HOST, PORT = "0.0.0.0", 9999

    # Init the TCP server object, bind it to the localhost on 9999 port
    tcp_server = socketserver.TCPServer((HOST, PORT), Handler_TCPServer)

    # Activate the TCP server.
    # To abort the TCP server, press Ctrl-C.
    tcp_server.serve_forever()
