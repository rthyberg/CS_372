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
        clientName = self.request.recv(headerSize).decode('utf-8').strip('\n') # strip and convert into string
        clientName = clientName[0:10]
        print("{} has connected!".format(clientName))
        serv = input("Please enter your name: ") # get server handle
        serverName = "".join(serv[0:10])
        self.mySend(serverName)
        while True: # get messages
            headerSize = self.myheader()
            self.data = self.myreceive(headerSize).decode('utf-8').strip('\n')
            if self.data == '\quit':
                print("{} has disconnected.\n".format(clientName))
                self.request.close()
                break
            print("{}> {}".format(clientName, self.data))
            msg = input("{} > ".format(serverName))
            self.mySend(msg)
            if msg == "\quit":
                print("Disconnecting from {}".format(clientName))
                break
        self.request.close()

    def myreceive(self, MSGLEN): # my receive all
        chunks = []
        bytes_recd = 0
        while bytes_recd < MSGLEN:
            chunk = self.request.recv((MSGLEN - bytes_recd))
            if chunk == b'':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
        return b''.join(chunks)

    def myheader(self): # my receive all for a consistent header of 2 BYTES
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

    def mySend(self, msg): # abstracted out sendall to clean up the handler
        header = (len(msg)).to_bytes(2,'big');
        bmsg = msg.encode('utf-8')
        self.request.sendall(header);
        self.request.sendall(bmsg);

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("USAGE: chatserve.py PORT")
        sys.exit(1)
    HOST, PORT = socketserver.socket.gethostname(), int(sys.argv[1]) # get cmdline args

    server = socketserver.TCPServer((HOST, PORT), MyTCPHandler) # start tcp server
    signal.signal(signal.SIGINT, signal_handler) # use sig int to quit
    print("Connection open on port " + HOST, PORT);
    server.serve_forever() # keep server up



