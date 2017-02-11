/*
** showip.c -- show IP addresses for a host given on the command line
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdint.h>
void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
int sendMsg(int, void*);
int recvMsg(int, void*, int);
int main(int argc, char *argv[])
{
    struct addrinfo hints, *res;
    int status, sockfd;

    if (argc != 3) {
        fprintf(stderr,"usage: showip hostname port\n");
        return 1;
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
       error("FAILED TO CONNECT TO CLIENT: ");
       }
    printf("CONNECTED TO HOST: %s on PORT: %s\n", argv[1], argv[2]);
    // vars to store our msgs
    char clientMsg[501];
    char hostMsg[501];
    char clientName[11]; // 1 extra than max then trim off \n
    char serverName[11];
    memset((char*) &clientName, '\0', sizeof clientName); // zero out memory
    printf("Please enter your name: ");
    fgets(clientName, 11, stdin);
    sendMsg(sockfd, clientName);
    memset((char*) &serverName, '\0', sizeof serverName);
    recvMsg(sockfd, serverName, 11);
    char* pos;
    if ((pos=strchr(clientName, '\n')) != NULL)
        *pos = '\0';
    printf("Connected with %s:\n", serverName);
    while(1) {
        memset((char*) &clientMsg, '\0', sizeof clientMsg);
        memset((char*) &hostMsg, '\0', sizeof hostMsg);
        printf("%s > ", clientName);
        fgets(clientMsg, 501, stdin);
        sendMsg(sockfd, clientMsg);
        if(strcmp(clientMsg, "\\quit\n") == 0) { // if we fgets a quit then we will return a 0 and break loop
            printf("Disconnecting from %s.\n", "Host");
            break;
        }
        recvMsg(sockfd, hostMsg, 501);
        if(strcmp(hostMsg, "\\quit") == 0) {
            printf("%s has disconnected.\n", serverName);
            break;
        }
        printf("%s > %s\n", serverName, hostMsg);
    }
    shutdown(sockfd,2);
    freeaddrinfo(res); // free the linked list
    return 0;
}
/* Loops recv so al data gets sent*/
int sendMsg(int sockfd, void* buffer) {
    int sent = 0; // holds the number of bytes sent
    int totalSent = 0; // hold the total of bytes sent
    unsigned short header = strlen(buffer);
    unsigned short nheader = htons(header); // we are gonna send a 16bit header representing
    do {                                               // the size of our msg
        sent = send(sockfd, &nheader, (sizeof(int16_t) - totalSent), 0); // sends the necessary amount of bytes
        totalSent = totalSent + sent;  // adds the number to total sent
    } while(totalSent < 2); // if we didnt send all the bytes then loop till we do
    sent = 0; // reset values for the message send
    totalSent = 0;
    do {
       sent = send(sockfd, buffer, (header-totalSent), 0); //same process but with msg
       totalSent = totalSent + sent;
    } while(totalSent < header);
    return 0;
}

/* A RecvAll function that also reads in a 2byte header before reading in a message*/
int recvMsg(int sockfd, void* buffer, int sizeofBuffer) {
    int recd = 0;
    int totalRecd = 0;
    unsigned short nheader, header;
    do {
        recd = recv(sockfd, &nheader, 2-totalRecd, 0); // loop till the header is received.. 2bytes unsigned shortl
        totalRecd = totalRecd + recd;
    } while(totalRecd < 2);
    recd = 0;
    totalRecd = 0;
    header = ntohs(nheader); // convert from network byte
    if(header > sizeofBuffer) { // if our header is bigger than our buffer then only read the size of the buffer
        header = sizeofBuffer;
    }
    do {
        recd = recv(sockfd, buffer, (header-totalRecd), 0); // loop recv until we read a specifed bytes from the header
        totalRecd = totalRecd + recd;
    } while(totalRecd < header);
    return 0;
}
