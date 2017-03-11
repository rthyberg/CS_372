/*
erver.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
int setUpTCPSocket(char*, char*);
int sendMsg(int, void*);
int recvMsg(int, void*, int);


#define BACKLOG 10     // how many pending connections queue will hold

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    if(argc != 2) {
        fprintf(stderr, "usage: ftserver port");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    printf("Hostname: %s\n", hostname);
    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            char msg [500];
            char* p;
            char* port = calloc(15, sizeof(char));
            char* hostname = calloc(500, sizeof(char));
            int datasock;
            const char* space = " ";
            memset((char*) &msg, '\0', sizeof msg);
            recvMsg(new_fd, msg, 500);

            p = strtok(msg, space);
            if(strcmp(p, "-l") == 0) {
                char * valid = "valid";
                sendMsg(new_fd, valid);
                port = strtok(NULL, space);
                hostname = strtok(NULL, "\n");
                datasock = setUpTCPSocket(hostname, port);
                sendMsg(datasock, "Heres some text for ya");
                close(datasock);
            } else if (strcmp(p, "-g") == 0) {
                printf("Got -g\n");
                char * valid = "valid";
                sendMsg(new_fd, valid);

            } else {
                printf("invalid command recieved\n");
                char * invalid = "Invalid command sent";
                sendMsg(new_fd, invalid);
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

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

int setUpTCPSocket(char* host, char* port) {
    struct addrinfo hints, *res;
    int status, sockfd;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if((status = getaddrinfo(host, port,  &hints, &res)) != 0) {
        fprintf(stderr, "ftserver: %s\n", gai_strerror(status));
        return 2;
    }
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        error("FAILED TO CONNECT TO CLIENT");
    }
    return sockfd;
}
