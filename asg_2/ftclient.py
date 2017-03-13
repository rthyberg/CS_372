import socket
import sys
import signal
import sys
import string
import struct
import os
import socket as so

def mybigheader(sockfd): # my receive all for a consistent header of 4 BYTES
    chunks = []
    bytes_recd = 0
    while bytes_recd < 4:
        chunk = sockfd.recv((4))
        if chunk == b'':
            raise RuntimeError("socket connection broken")
        chunks.append(chunk)
        bytes_recd = bytes_recd + len(chunk)
        bytes_recd += 1
    mynum = int.from_bytes(b"".join(chunks),'big')
    return mynum

def myheader(sockfd): # my receive all for a consistent header of 2 BYTES
    chunks = []
    bytes_recd = 0
    while bytes_recd < 2:
        chunk = sockfd.recv((2))
        if chunk == b'':
            raise RuntimeError("socket connection broken")
        chunks.append(chunk)
        bytes_recd = bytes_recd + len(chunk)
        bytes_recd += 1
    mynum = int.from_bytes(b"".join(chunks),'big')
    return mynum

def recvAll(sockfd, MSGLEN): # my receive all that takes in a header argument called MSGLEN
        chunks = []
        bytes_recd = 0
        while bytes_recd < MSGLEN:
            chunk = sockfd.recv((MSGLEN - bytes_recd))
            if chunk == b'':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
        return b''.join(chunks)

if __name__ == "__main__":
    argl = len(sys.argv)
    if argl > 4 and argl < 7:
        if sys.argv[3] == "-l" and argl == 5: # parse the cmdline arguments depending on the command given.
            HOST, PORT, COM, DPORT = sys.argv[1], int(sys.argv[2]), sys.argv[3], int(sys.argv[4])
            FILENAME = ""
        elif sys.argv[3] == "-g" and argl == 6:
            HOST, PORT, COM, FILENAME, DPORT = sys.argv[1], int(sys.argv[2]), sys.argv[3], sys.argv[4], int(sys.argv[5])
        elif argl == 5:
            print("USAGE: ftclient.py HOST CPORT [-l|-g FILENAME] DPORT")
            HOST, PORT, COM, DPORT = sys.argv[1], int(sys.argv[2]), sys.argv[3], int(sys.argv[4])
            FILENAME = ""
        elif argl == 6:
            print("USAGE: ftclient.py HOST CPORT [-l|-g FILENAME] DPORT")
            HOST, PORT, COM, FILENAME, DPORT = sys.argv[1], int(sys.argv[2]), sys.argv[3], sys.argv[4], int(sys.argv[5])

    else:
        print("USAGE: ftclient.py HOST CPORT [-l|-g FILENAME] DPORT")
        sys.exit(1)
    data = " "
    HOSTNAME = socket.gethostname()
    # Create a socket (SOCK_STREAM means a TCP socket)
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    # Connect to server and send data
        sock.connect((HOST, PORT))
        msg = COM + " " + FILENAME + " " + str(DPORT) + " " + HOSTNAME + "\n"
        bmsg = msg.encode('utf-8')
        header = (len(msg)).to_bytes(2, 'big')
        sock.sendall(header)
        sock.sendall(bmsg)
        # Receive data from the server and shut down
        header = myheader(sock)
        received1 = str(recvAll(sock, header), "utf-8")

    if (COM == "-l" and received1 == "valid") or  (COM == "-g" and received1 == "valid"): # check if response is needed and if valid
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((HOSTNAME, DPORT))
            s.listen(1)
            conn, addr = s.accept()
            with conn:
                print('Connected by', addr)
                if COM == "-l": # get dir
                    header = myheader(conn)
                    received = str(recvAll(conn, header), "utf-8")
                    conn.close()
                elif COM == "-g": # get file
                    header = mybigheader(conn)
                    received = recvAll(conn, header)
                    conn.close

    if COM == "-g" and received1 == "valid":
        if os.path.isfile(FILENAME): # if copy then call write as Copy of
            FILENAME = "Copy-of-"+ FILENAME;
        fp = open(FILENAME, "wb")
        fp.write(received)
    elif COM == "-l" and received1 == "valid":
        print("Current Dir: {}".format(received)) # show dir
    else:
        print("Server Response: {}".format(received1)) # print error messages from server
