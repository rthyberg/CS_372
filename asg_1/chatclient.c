#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc , char *argv[])
{
    int socketFD, portNumber, charsWritten;
    char* ip;
    char check[255];
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    if(argc == 3) {
        ip = argv[1];
        portNumber = atoi(argv[2]);
    } else{
       printf("USAGE: %s HOSTNAME PORT_NUM\n", argv[0]);
       exit(1);
    }
    // Set up the address struct for this process
    memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverHostInfo = gethostbyname("localhost"); // Convert the machine name into special form of address
    if(serverHostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(2);
    }
     memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Co    py in the address

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
        exit(2);
    }
 // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to address
        error("CLIENT: ERROR connecting");
        exit(2);
    }
     // handshake
    charsWritten = send(socketFD, "otp_enc", 8, 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(check)) fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");

    return 0;
}
