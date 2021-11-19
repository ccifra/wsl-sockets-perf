#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <hvsocket.h>
#include <windows.h>
#include <test_config.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Rpcrt4.lib")

#ifndef AF_HYPERV
#define AF_HYPERV 34
#endif

#define BUFF_SIZE 400
// int optval = 1;
//#define DEFAULT_BUFLEN 512
#define DEFAULT_TCP_PORT "50051"


using namespace std;
using timeVector = std::vector<std::chrono::microseconds>;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void Log(int ret, const char* function)
{
    if (ret == 0)
    {
        printf("%s success\n", function);
    }
    else
    {
        printf("%s error: %d\n", function, WSAGetLastError());
        error(0);
    }
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
            error(0);
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
void WriteLatencyData(timeVector times, const string& fileName)
{
    auto iterations = times.size();

    {
        std::ofstream fout;
        fout.open("xaxis");
        for (int x=0; x<iterations; ++x)
        {
            fout << (x+1) << std::endl;
        }
        fout.close();
    }

    {
        std::ofstream fout;
        fout.open(fileName);
        for (auto i : times)
        {
            fout << i.count() << std::endl;
        }
        fout.close();
    }

    std::sort(times.begin(), times.end());
    auto min = times.front();
    auto max = times.back();
    auto median = *(times.begin() + iterations / 2);
    
    double average = times.front().count();
    for (auto i : times)
        average += (double)i.count();
    average = average / iterations;

    cout << "End Test" << endl;
    cout << "Min: " << min.count() << endl;
    cout << "Max: " << max.count() << endl;
    cout << "Median: " << median.count() << endl;
    cout << "Average: " << average << endl;
    cout << endl;
}

// int optval = 1;
// #define DEFAULT_BUFLEN 512
// #define DEFAULT_PORT "50051"

// int main(int argc, char *argv[])
// {
//     WSADATA wsaData;
//     SOCKET ConnectSocket = INVALID_SOCKET;
//     struct addrinfo *result = NULL,
//                     *ptr = NULL,
//                     hints;
//     const char *sendbuf = "this is a test";
//     char recvbuf[DEFAULT_BUFLEN];
//     int iResult;
//     int recvbuflen = DEFAULT_BUFLEN;
    
//     // Initialize Winsock
//     iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
//     if (iResult != 0) {
//         printf("WSAStartup failed with error: %d\n", iResult);
//         return 1;
//     }

//     ZeroMemory( &hints, sizeof(hints) );
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_protocol = IPPROTO_TCP;

//     // Resolve the server address and port
//     iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
//     if ( iResult != 0 ) {
//         printf("getaddrinfo failed with error: %d\n", iResult);
//         WSACleanup();
//         return 1;
//     }

//     // Attempt to connect to an address until one succeeds
//     for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

//         // Create a SOCKET for connecting to server
//         ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
//             ptr->ai_protocol);
//         if (ConnectSocket == INVALID_SOCKET) {
//             printf("socket failed with error: %ld\n", WSAGetLastError());
//             WSACleanup();
//             return 1;
//         }

//         // Connect to server.
//         iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
//         if (iResult == SOCKET_ERROR) {
//             closesocket(ConnectSocket);
//             ConnectSocket = INVALID_SOCKET;
//             continue;
//         }
//         break;
//     }

//     freeaddrinfo(result);

//     if (ConnectSocket == INVALID_SOCKET) {
//         printf("Unable to connect to server!\n");
//         WSACleanup();
//         return 1;
//     }

// #ifndef _WIN32    
//     // sched_param schedParam;
//     // schedParam.sched_priority = 95;
//     // sched_setscheduler(0, SCHED_FIFO, &schedParam);

//     // cpu_set_t cpuSet;
//     // CPU_ZERO(&cpuSet);
//     // CPU_SET(2, &cpuSet);
//     // sched_setaffinity(0, sizeof(cpu_set_t), &cpuSet);
// #endif

//     int portno, n;
//     sockaddr_in serv_addr;
//     hostent *server;

//     // WSADATA wsaData;
//     // WSAStartup(MAKEWORD(2,2), &wsaData);

//     char buffer[256];
//     if (argc < 3) {
//        fprintf(stderr,"usage %s hostname port\n", argv[0]);
//        exit(0);
//     }
//     portno = atoi(argv[2]);
//     // sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//     // if (sockfd < 0) 
//     // {
//     //     error("ERROR opening socket");
//     // }

//     // server = gethostbyname(argv[1]);
//     // if (server == NULL) {
//     //     fprintf(stderr,"ERROR, no such host\n");
//     //     exit(0);
//     // }
//     // memset((char *) &serv_addr, 0, sizeof(serv_addr));
//     // serv_addr.sin_family = AF_INET;
//     // memcpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
//     // serv_addr.sin_port = htons(portno);
//     // if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
//     // {
//     //     error("ERROR connecting");
//     // }
//     printf("RT Please enter the message: ");
//     memset(buffer, 0, 256);
    
//     fgets(buffer, 255, stdin);

//     n = send(ConnectSocket, buffer, strlen(buffer), 0);
//     if (n < 0) 
//     {
//          error("ERROR writing to socket");
//     }
//     memset(buffer, 0, 256);
    
//     timeVector times;
//     times.reserve(300000);
    
//     double* doubles = (double*)malloc(32 * sizeof(double));
//     for (int x=0; x<100; ++x)
//     {
//         WriteToSocket(ConnectSocket, doubles, 1 * sizeof(double));
//         ReadFromSocket(ConnectSocket, doubles, 1 * sizeof(double));
//     }
//     for (int x=0; x<300000; ++x)
//     {
//         auto start = std::chrono::steady_clock::now();
//         WriteToSocket(ConnectSocket, doubles, 1 * sizeof(double));
//         ReadFromSocket(ConnectSocket, doubles, 1 * sizeof(double));
//         auto end = std::chrono::steady_clock::now();
//         auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//         times.emplace_back(elapsed);
//     }
//     std::cout << "Done" << std::endl;
//     //std::cout << "It took me " << elapsed.count() << " microseconds." << std::endl;
//     WriteLatencyData(times, "socketlatency.txt");
//     printf("read: %d\n", n);
//     closesocket(ConnectSocket);
//     return 0;
// }

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunLatencyTest(SOCKET clientSocket)
{
    std::cout << "Start Latency Test." << std::endl;    

    timeVector times;
    times.reserve(300000);
    
    double* doubles = (double*)malloc(32 * sizeof(double));
    for (int x=0; x<100; ++x)
    {
        WriteToSocket(clientSocket, doubles, 1 * sizeof(double));
        ReadFromSocket(clientSocket, doubles, 1 * sizeof(double));
    }
    for (int x=0; x<300000; ++x)
    {
        auto start = std::chrono::steady_clock::now();
        WriteToSocket(clientSocket, doubles, 1 * sizeof(double));
        ReadFromSocket(clientSocket, doubles, 1 * sizeof(double));
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        times.emplace_back(elapsed);
    }
    std::cout << "Done" << std::endl;
    //std::cout << "It took me " << elapsed.count() << " microseconds." << std::endl;
    WriteLatencyData(times, "socketlatency.txt");
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunHandshakeThroughputTest(SOCKET clientSocket)
{    
    int iterations = NUM_THROUGHPUT_ITERATIONS;
    double* doubles = (double*)malloc(200000 * sizeof(double));
    auto start = std::chrono::steady_clock::now();
    for (int x=0; x<iterations; ++x)
    {
        WriteToSocket(clientSocket, doubles, 1 * sizeof(double));
        ReadFromSocket(clientSocket, doubles, 200000 * sizeof(double));
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Done" << std::endl;
    std::cout << "It took me " << elapsed.count() << " microseconds." << std::endl;

    double totalBytes = 200000.0 * iterations * 8.0;
    double totalMB = totalBytes / (1024.0 * 1024.0);
    std::cout << "Total MB: " << totalMB << std::endl;

    double totalSeconds = elapsed.count() / (1000.0 * 1000.0);
    std::cout << "Total seconds: " << totalSeconds << std::endl;
    double MBPerSecond = totalMB / totalSeconds;
    std::cout << "Handshake Throughput " << MBPerSecond << " MB/S." << std::endl;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunNoHandshakeThroughputTest(SOCKET clientSocket)
{    
    int iterations = NUM_THROUGHPUT_ITERATIONS;
    double* doubles = (double*)malloc(200000 * sizeof(double));
    WriteToSocket(clientSocket, doubles, 1 * sizeof(double));
    auto start = std::chrono::steady_clock::now();
    for (int x=0; x<iterations; ++x)
    {
        ReadFromSocket(clientSocket, doubles, 200000 * sizeof(double));
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Done" << std::endl;
    std::cout << "It took me " << elapsed.count() << " microseconds." << std::endl;

    double totalBytes = 200000.0 * iterations * 8.0;
    double totalMB = totalBytes / (1024.0 * 1024.0);

    double totalSeconds = elapsed.count() / (1000.0 * 1000.0);
    double MBPerSecond = totalMB / totalSeconds;
    std::cout << "No Handshake Throughput " << MBPerSecond << " MB/S." << std::endl;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SOCKET ConnectTCPSocket()
{    
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;
    
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("localhost", DEFAULT_TCP_PORT, &hints, &result);
    if ( iResult != 0 )
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        error(0);
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    return ConnectSocket;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SOCKET ConnectVSocket(std::string VmId)
{    
    SOCKET Sock = socket(AF_HYPERV, SOCK_STREAM, HV_PROTOCOL_RAW);
    if (Sock > 0)
    {
        printf("server socket: %lld\n", Sock);
    }
    else
    {
        printf("socket error: %d\n", WSAGetLastError());
        error(0);
    }

    SOCKADDR_HV addr = { 0 };
    addr.Family = AF_HYPERV;
    //struct __declspec(uuid("AD64DA96-03EC-4596-B7D7-453D0C3E55E3")) ServerVsockTemplate {};
    //struct __declspec(uuid(VmId)) ServerVsockTemplate {};
    UuidFromString((RPC_CSTR)VmId.c_str(), &addr.VmId); // __uuidof(ServerVsockTemplate);
    //addr.VmId = __uuidof(ServerVsockTemplate);

    std::cout << "VM ID: " << addr.VmId.Data1 << std::endl;

    memcpy(&addr.ServiceId, &HV_GUID_VSOCK_TEMPLATE, sizeof(addr.ServiceId));
    unsigned long Port = 50053;
    addr.ServiceId.Data1 = Port;
    addr.Reserved = 0;
    int ret = connect(Sock, (struct sockaddr*)&addr, sizeof addr);
    Log(ret, "connect");
    return Sock;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int main(int argc, char **argv)
{
    WSADATA wdata;
    int ret = WSAStartup(MAKEWORD(2,2), &wdata);
    if (ret != 0)
    {
        printf("WSAStartup error: %d\n", ret);
        error(0);
    }

    // char msg[BUFF_SIZE];
    // while (1)
    // {
    //     memset(msg, 0, sizeof msg);
    //     printf("Enter message: ");
    //     scanf("%s", msg);
    //     ret = send(Sock, msg, strlen(msg), 0);
    //     if (ret < 0)
    //     {
    //         printf("send error: %d\n", WSAGetLastError());
    //         break;
    //     }
    // }

    SOCKET Sock;
    if (argc > 1 && std::string(argv[1]) == "-vsock")
    {
        if (argc > 2)
        {
            std::cout << "Connecting to VSock" << std::endl;
            Sock = ConnectVSocket(argv[2]);
        }
    }
    else
    {
        std::cout << "Connecting to TCP Socket" << std::endl;
        Sock = ConnectTCPSocket();
    }

    if (Sock > 0)
    {
        RunLatencyTest(Sock);
        RunHandshakeThroughputTest(Sock);
        RunNoHandshakeThroughputTest(Sock);
    }

    /* cleanup */
    if (Sock > 0)
    {
        closesocket(Sock);
    }
    return 0;
}
