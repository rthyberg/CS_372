import socketserver
import signal
import sys
import string
import struct
import socket as so
def signal_handler(signal, frame):
    print('Press Ctrl+C')
    sys.exit(0)


class MyTCPHandler(socketserver.StreamRequestHandler):

    def handle(self):
        # self.rfile is a file-like object created by the handler;
        # we can now use e.g. readline() instead of raw recv() calls
        headerSize = self.myheader()
        self.data = ''
        clientName = self.request.recv(headerSize).decode('utf-8').strip('\n')
        clientName = clientName[0:10]
        print("{} has connected!".format(clientName))
        serv = input("Please enter your name: ")
        serverName = "".join(serv[0:10])
        self.mySend(serverName)
        while True:
            headerSize = self.myheader()
            self.data = self.myreceive(headerSize).decode('utf-8').strip('\n')
            if self.data == '\quit':
                print("{} has disconnected.\n".format(clientName))
                self.request.close()
                break
            print("{}> {}".format(clientName, self.data))
            msg = input("{} > ".format(serverName))
            self.mySend(msg)
            print(msg)
            if msg == "\quit":
                print("Disconnecting from {}".format(clientName))
                break
            # Likewise, self.wfile is a file-like object used to write back
            # to the client
            #self.wfile.write(self.data.upper())
        self.request.close()

    def myreceive(self, MSGLEN):
        chunks = []
        bytes_recd = 0
        while bytes_recd < MSGLEN:
            chunk = self.request.recv((MSGLEN - bytes_recd))
            if chunk == b'':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
        return b''.join(chunks)

    def myheader(self):
        chunks = []
        bytes_recd = 0
        while bytes_recd < 2:
            chunk = self.request.recv((2))
            if chunk == b'':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
            bytes_recd += 1
        mynum = int.from_bytes(b"".join(chunks),'big')
        return mynum

    def mySend(self, msg):
        header = (len(msg)).to_bytes(2,'big');
        bmsg = msg.encode('utf-8')
        self.request.sendall(header);
        self.request.sendall(bmsg);

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("USAGE: chatserve.py PORT")
        sys.exit(1)
    HOST, PORT = socketserver.socket.gethostname(), int(sys.argv[1])

    # Create the server, binding to localhost on port 9999
    server = socketserver.TCPServer((HOST, PORT), MyTCPHandler)
    signal.signal(signal.SIGINT, signal_handler)
    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    print("Connection open on port " + HOST, PORT);
    server.serve_forever()



