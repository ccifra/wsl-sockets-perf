//---------------------------------------------------------------------
// Windows server sockets performance test app
//---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sys/types.h> 
#include <winsock2.h>
#include <test_config.h>

#pragma comment(lib, "Ws2_32.lib")

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
        int written = send(socketfd, (const char*)buffer, remainingBytes, 0);
        if (written < 0)
        {
            std::cout << "Error writing to buffer";
        }
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
        int n;
        do
        {
            n = recv(socket, (char*)buffer, totalToRead, 0);
        } while (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));

        if (n < 0)
        {
            std::cout << "Failed To read." << std::endl;
        }
        totalToRead -= n;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunLatencyTest(int connectSocket)
{    
    double* d = (double*)malloc(32 * sizeof(double));
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
void RunHandshakeThroughputTest(int connectSocket)
{    
    double* doubles = (double*)malloc(200000 * sizeof(double));
    for (int x=0; x<NUM_THROUGHPUT_ITERATIONS; ++x)
    {
        ReadFromSocket(connectSocket, doubles, 1 * sizeof(double));
        WriteToSocket(connectSocket, doubles, 200000 * sizeof(double));
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunNoHandshakeThroughputTest(int connectSocket)
{    
    double* doubles = (double*)malloc(200000 * sizeof(double));
    ReadFromSocket(connectSocket, doubles, 1 * sizeof(double));
    for (int x=0; x<NUM_THROUGHPUT_ITERATIONS; ++x)
    {
        WriteToSocket(connectSocket, doubles, 200000 * sizeof(double));
    }
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
int main(int argc, char *argv[])
{

    int sockfd, newsockfd;
    int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    // create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
       error("ERROR opening socket");
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    int portno = 50051;

    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
       error("ERROR on binding");
    }

    listen(sockfd, 5);

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
    { 
        error("ERROR on accept");
    }

    std::cout << "Connection!" << std::endl;

    RunLatencyTest(newsockfd);     
    RunHandshakeThroughputTest(newsockfd);
    RunNoHandshakeThroughputTest(newsockfd);

    closesocket(newsockfd);
    closesocket(sockfd);
    return 0; 
}
