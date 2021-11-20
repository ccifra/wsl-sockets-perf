//---------------------------------------------------------------------
// Linux server sockets performance test app
//---------------------------------------------------------------------
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <linux/vm_sockets.h>
#include <netinet/in.h>
#include "test_config.h"

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static int sockfd;

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
            std::cout << "Error writing to buffer" << std::endl;
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
        int n = 0;
        do
        {
            n = recv(socket, (char*)buffer, totalToRead, 0); //MSG_DONTWAIT);
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
    for (int x=0; x<NUM_LATENCY_ITERATIONS; ++x)
    {
       ReadFromSocket(connectSocket, d, 1 * sizeof(double));
       WriteToSocket(connectSocket, d, 1 * sizeof(double));
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunHandshakedThroughputTest(int connectSocket)
{    
    double* doubles = (double*)malloc(DOUBLES_PER_ITERATION * sizeof(double));
    for (int x=0; x<NUM_THROUGHPUT_ITERATIONS; ++x)
    {
        ReadFromSocket(connectSocket, doubles, 1 * sizeof(double));
        WriteToSocket(connectSocket, doubles, DOUBLES_PER_ITERATION * sizeof(double));
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunNoHandshakedThroughputTest(int connectSocket)
{    
    double* doubles = (double*)malloc(DOUBLES_PER_ITERATION * sizeof(double));
    ReadFromSocket(connectSocket, doubles, 1 * sizeof(double));
    for (int x=0; x<NUM_THROUGHPUT_ITERATIONS; ++x)
    {
        WriteToSocket(connectSocket, doubles, DOUBLES_PER_ITERATION * sizeof(double));
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int AcceptVSock()
{    
    sockfd = socket(AF_VSOCK, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cout << "socket error: " << strerror(errno) << std::endl;
    }
    else
    {
        std::cout << "socket: " << sockfd << std::endl;
    }
    struct sockaddr_vm addr = { 0 };
    addr.svm_family = AF_VSOCK;
    addr.svm_port = TEST_VSOCK_PORT; //VMADDR_PORT_ANY;
    addr.svm_cid = VMADDR_CID_ANY;
    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof addr);
    if (ret < 0)
    {
        std::cout << "bind error: " << strerror(errno) << std::endl;
    }
    socklen_t addrlen = sizeof addr;
    ret = getsockname(sockfd, (struct sockaddr*)&addr, &addrlen);
    if (ret < 0)
    {
        std::cout << "getsockname error: " << strerror(errno) << std::endl;
    }
    else
    {
        std::cout << "getsockname port: " << addr.svm_port << std::endl;
    }
    ret = listen(sockfd, 1);
    if (ret < 0)
    {
        std::cout << "listen error: " << strerror(errno) << std::endl;
    }
    int connectSocket = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
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

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(TEST_TCP_PORT);

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
int main(int argc, char **argv)
{
    int connectSocket = 0;
    if (argc > 1 && std::string(argv[1]) == "-vsock")
    {
        std::cout << "Listening on VSock" << std::endl;
        connectSocket = AcceptVSock();
    }
    else
    {
        std::cout << "Listening on TCP Socket" << std::endl;
        connectSocket = AcceptTCPSocket();
    }

    if (connectSocket < 0)
    {
        std::cout << "accept error: " << strerror(errno) << std::endl;
    }
    else
    {
        std::cout << "client socket: " << connectSocket << std::endl;
    }

    RunLatencyTest(connectSocket);
    RunHandshakedThroughputTest(connectSocket);
    RunNoHandshakedThroughputTest(connectSocket);

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
