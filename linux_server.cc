#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sys/socket.h>
#include <linux/vm_sockets.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/tcp.h>

#define BUFF_SIZE 400

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void WriteToSocket(int socketfd, void* buffer, int numBytes)
{
    auto remainingBytes = numBytes;
    while (remainingBytes > 0)
    {
        int written = send(socketfd, (char*)buffer, remainingBytes, 0);
        if (written < 0)
        {
            std::cout << "Error writing to buffer";
        }
        //std::cout << "writing data" << std::endl;
        remainingBytes -= written;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void ReadFromSocket(int socket, void* buffer, int count)
{
    auto totalToRead = count;
    while (totalToRead > 0)
    {        
        int n = 0;
        do
        {
            n = recv(socket, (char*)buffer, totalToRead, 0); //MSG_DONTWAIT);
            // if (n < 0)
            // {
            //     if (errno == EAGAIN)
            //     {
            //         std::cout << "No data Try Again" << std::endl;
            //     }
            //     std::cout << "No data" << std::endl;
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

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunLatencyTest(int connectSocket)
{    
    double* d = (double*)malloc(32 * sizeof(double));
    // This send() function sends the 13 bytes of the string to the new socket
    for (int x=0; x<100; ++x)
    {
       ReadFromSocket(connectSocket, d, 1 * sizeof(double));
       WriteToSocket(connectSocket, d, 1 * sizeof(double));
    }
    for (int x=0; x<300000; ++x)
    {
       ReadFromSocket(connectSocket, d, 1 * sizeof(double));
       WriteToSocket(connectSocket, d, 1 * sizeof(double));
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunThroughputTest(int connectSocket)
{    
    double* doubles = (double*)malloc(200000 * sizeof(double));
    for (int x=0; x<1000; ++x)
    {
        ReadFromSocket(connectSocket, doubles, 1 * sizeof(double));
        WriteToSocket(connectSocket, doubles, 200000 * sizeof(double));
    }
}

static int sockfd;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int AcceptVSock()
{    
    sockfd = socket(AF_VSOCK, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("socket error: %s\n", strerror(errno));
    }
    else
    {
        printf("socket: %d\n", sockfd);
    }
    struct sockaddr_vm addr = { 0 };
    addr.svm_family = AF_VSOCK;
    addr.svm_port = 50053; //VMADDR_PORT_ANY;
    addr.svm_cid = VMADDR_CID_ANY;
    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof addr);
    if (ret < 0)
    {
        printf("bind error: %s\n", strerror(errno));
    }
    socklen_t addrlen = sizeof addr;
    ret = getsockname(sockfd, (struct sockaddr*)&addr, &addrlen);
    if (ret < 0)
    {
        printf("getsockname error: %s\n", strerror(errno));
    }
    else
    {
        printf("getsockname port: %d\n", addr.svm_port);
    }
    ret = listen(sockfd, 1);
    if (ret < 0)
    {
        printf("listen error: %s\n", strerror(errno));
    }
    int connectSocket = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
    if (connectSocket < 0)
    {
        printf("accept error: %s\n", strerror(errno));
    }
    else
    {
        printf("client socket: %d\n", connectSocket);
    }
    return connectSocket;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int AcceptTCPSocket()
{
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
       error("ERROR opening socket");
    }

    auto portno = 50051;

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
       error("ERROR on binding");
    }

    listen(sockfd, 5);

    unsigned int clilen = sizeof(cli_addr);
    auto newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
    { 
        error("ERROR on accept");
    }     
    return newsockfd; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int main(void)
{
    //int connectSocket = AcceptVSock();
    int connectSocket = AcceptTCPSocket();
    RunLatencyTest(connectSocket);
    RunThroughputTest(connectSocket);

    // char msg[BUFF_SIZE];

    // do
    // {
    //     memset(msg, 0, sizeof msg);
    //     ret = recv(csockfd, msg, BUFF_SIZE, 0);
    //     if(ret > 0)
    //         printf("%s\n", msg);
    //     else if (ret == 0)
    //         printf("server closing...\n");
    //     else
    //         break;

    // } while (ret > 0);

    // cleanup
    if (sockfd > 0)
    {
        close(sockfd);
    }
    if (connectSocket > 0)
    {
        close(connectSocket);
    }
    return 0;
}
