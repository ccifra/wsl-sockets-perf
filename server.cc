#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <ostream>
//#include <unistd.h>
#include <sys/types.h> 
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <linux/tcp.h>

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void WriteToSocket(int socketfd, void* buffer, int numBytes)
{
    auto remainingBytes = numBytes;
    while (remainingBytes > 0)
    {
        int written = send(socketfd, (const char*)buffer, remainingBytes, 0);
        if (written < 0)
        {
            std::cout << "Error writing to buffer";
        }
        //std::cout << "writing data" << std::endl;
        remainingBytes -= written;
    }
}

void ReadFromSocket(int socket, void* buffer, int count)
{
    auto totalToRead = count;
    while (totalToRead > 0)
    {        
        int n;
        do
        {
            n = recv(socket, (char*)buffer, totalToRead, 0); //MSG_DONTWAIT);
            // if (n < 0)
            // {
            //     // if (errno == EAGAIN)
            //     // {
            //     //     std::cout << "No data Try Again" << std::endl;
            //     // }
            //     // std::cout << "No data" << std::endl;
            // }
        } while (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));

        if (n < 0)
        {
            std::cout << "Failed To read." << std::endl;
        }
        // std::cout << "Got Data: " << n << " Total: " << totalToRead << std::endl;
        totalToRead -= n;
    }
}

int optval = 1;

int main(int argc, char *argv[])
{
#ifndef _WIN32    
    // sched_param schedParam;
    // schedParam.sched_priority = 95;
    // sched_setscheduler(0, SCHED_FIFO, &schedParam);

    // cpu_set_t cpuSet;
    // CPU_ZERO(&cpuSet);
    // CPU_SET(2, &cpuSet);
    // sched_setaffinity(0, sizeof(cpu_set_t), &cpuSet);
#endif

    int sockfd, newsockfd, portno;
    int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    // create a socket
    // socket(int domain, int type, int protocol)
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
       error("ERROR opening socket");
    }

    // clear address structure
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);

    /* setup the host_addr structure for use in bind call */
    // server byte order
    serv_addr.sin_family = AF_INET;  
    // automatically be filled with current host's IP address
    serv_addr.sin_addr.s_addr = INADDR_ANY;  

    // convert short integer value for port must be converted into network byte order
    serv_addr.sin_port = htons(portno);

    // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
    // bind() passes file descriptor, the address structure, 
    // and the length of the address structure
    // This bind() call will bind  the socket to the current IP address on port, portno
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
       error("ERROR on binding");
    }

    // This listen() call tells the socket to listen to the incoming connections.
    // The listen() function places all incoming connection into a backlog queue
    // until accept() call accepts the connection.
    // Here, we set the maximum size for the backlog queue to 5.
    listen(sockfd, 5);

    // The accept() call actually accepts an incoming connection
    clilen = sizeof(cli_addr);

    // This accept() function will write the connecting client's address info 
    // into the the address structure and the size of that structure is clilen.
    // The accept() returns a new socket file descriptor for the accepted connection.
    // So, the original socket file descriptor can continue to be used 
    // for accepting new connections while the new socker file descriptor is used for
    // communicating with the connected client.
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
    { 
        error("ERROR on accept");
    }     

    printf("RT Test Test Test\n");
    //  printf("server: got connection from %s port %d\n",
    //         inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));


    double* d = (double*)malloc(32 * sizeof(double));
    // This send() function sends the 13 bytes of the string to the new socket
    for (int x=0; x<100; ++x)
    {
       ReadFromSocket(newsockfd, d, 1 * sizeof(double));
       WriteToSocket(newsockfd, d, 1 * sizeof(double));
    }
    for (int x=0; x<300000; ++x)
    {
       ReadFromSocket(newsockfd, d, 1 * sizeof(double));
       WriteToSocket(newsockfd, d, 1 * sizeof(double));
    }

    memset(buffer, 0, 256);

    n = recv(newsockfd, buffer, 255, 0);
    if (n < 0)
    {
        error("ERROR reading from socket");
    }
    printf("Here is the message: %s\n",buffer);

    closesocket(newsockfd);
    closesocket(sockfd);
    return 0; 
}
