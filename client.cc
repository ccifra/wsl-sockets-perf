//---------------------------------------------------------------------
// Windows client sockets performance test app
//---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Rpcrt4.lib")

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifndef AF_HYPERV
    #define AF_HYPERV 34
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
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
        std::cout << function << " success" << std::endl;
    }
    else
    {
        std::cout << function << " error: " << WSAGetLastError() << std::endl;
        error(0);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void WriteToSocket(SOCKET socketfd, void* buffer, int numBytes)
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
        remainingBytes -= written;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void ReadFromSocket(SOCKET socket, void* buffer, int count)
{
    auto totalToRead = count;
    while (totalToRead > 0)
    {        
        int n = 0;
        do
        {
            n = recv(socket, (char*)buffer, totalToRead, 0);
        } while (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));

        if (n < 0)
        {
            std::cout << "Failed To read." << std::endl;
            error(0);
        }
        totalToRead -= n;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void WriteLatencyData(timeVector times, const std::string& fileName)
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
    
    double average = (double)times.front().count();
    for (auto i : times)
    {
        average += (double)i.count();
    }
    average = average / iterations;

    std::cout << "End Test" << std::endl;
    std::cout << "Min: " << min.count() << std::endl;
    std::cout << "Max: " << max.count() << std::endl;
    std::cout << "Median: " << median.count() << std::endl;
    std::cout << "Average: " << average << std::endl;
    std::cout << std::endl;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunLatencyTest(SOCKET clientSocket)
{
    std::cout << "Start Latency Test." << std::endl;    

    timeVector times;
    times.reserve(NUM_LATENCY_ITERATIONS);
    
    double* doubles = (double*)malloc(32 * sizeof(double));
    for (int x=0; x<100; ++x)
    {
        WriteToSocket(clientSocket, doubles, 1 * sizeof(double));
        ReadFromSocket(clientSocket, doubles, 1 * sizeof(double));
    }
    for (int x=0; x<NUM_LATENCY_ITERATIONS; ++x)
    {
        auto start = std::chrono::steady_clock::now();
        WriteToSocket(clientSocket, doubles, 1 * sizeof(double));
        ReadFromSocket(clientSocket, doubles, 1 * sizeof(double));
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        times.emplace_back(elapsed);
    }
    std::cout << "Done" << std::endl;
    WriteLatencyData(times, "socketlatency.txt");
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunHandshakeThroughputTest(SOCKET clientSocket)
{    
    int iterations = NUM_THROUGHPUT_ITERATIONS;
    double* doubles = (double*)malloc(DOUBLES_PER_ITERATION * sizeof(double));
    auto start = std::chrono::steady_clock::now();
    for (int x=0; x<iterations; ++x)
    {
        WriteToSocket(clientSocket, doubles, 1 * sizeof(double));
        ReadFromSocket(clientSocket, doubles, DOUBLES_PER_ITERATION * sizeof(double));
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Done" << std::endl;
    std::cout << "It took me " << elapsed.count() << " microseconds." << std::endl;

    double totalBytes = (double)DOUBLES_PER_ITERATION * iterations * 8.0;
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
    double* doubles = (double*)malloc(DOUBLES_PER_ITERATION * sizeof(double));
    WriteToSocket(clientSocket, doubles, 1 * sizeof(double));
    auto start = std::chrono::steady_clock::now();
    for (int x=0; x<iterations; ++x)
    {
        ReadFromSocket(clientSocket, doubles, DOUBLES_PER_ITERATION * sizeof(double));
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Done" << std::endl;
    std::cout << "It took me " << elapsed.count() << " microseconds." << std::endl;

    double totalBytes = (double)DOUBLES_PER_ITERATION * iterations * 8.0;
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
    iResult = getaddrinfo("localhost", TEST_TCP_PORT_STR, &hints, &result);
    if ( iResult != 0 )
    {
        std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
        error(0);
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
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
        std::cout << "Unable to connect to server!" << std::endl;
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
        std::cout << "server socket: " << Sock << std::endl;
    }
    else
    {
        std::cout << "socket error: " << WSAGetLastError() << std::endl;
        error(0);
    }

    SOCKADDR_HV addr = { 0 };
    addr.Family = AF_HYPERV;
    UuidFromString((RPC_CSTR)VmId.c_str(), &addr.VmId);
    std::cout << "VM ID: " << addr.VmId.Data1 << std::endl;

    memcpy(&addr.ServiceId, &HV_GUID_VSOCK_TEMPLATE, sizeof(addr.ServiceId));
    unsigned long Port = TEST_VSOCK_PORT;
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
        std::cout << "WSAStartup error: " << ret << std::endl;
        error(0);
    }

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

    // cleanup
    if (Sock > 0)
    {
        closesocket(Sock);
    }
    return 0;
}
