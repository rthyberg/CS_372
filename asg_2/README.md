INSTRUCTIONS:
Run on flip.engr.oregonstate.edu

make the bin using the make file.
./ftserver PORT to run the server
ftserver will return the hostname into the terminal, use this in the python3 client
python3 ftclient.py HOSTNAME PORT [-l|-g FILENAME] DPORT

About:
The code base for ftserver.c was based off the stream server from beej guide to socket programming. I reused
a lot of the code from the last assignment when it comes to recvAll and sendAll. I made slight modication to the header for a file transfer that allows
a (sizeof long) bytes to be transfered. This allows a about a 4 gig file to be transfered asssuming their is enough system memory. I did not test this big of a file.

The python code was based off the python3 documentation. This time i moved away from using socketserver. I used just the socket module. I reused the code from the last
project but adjusted it to use the socket that is passed in.

