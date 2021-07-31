#include "TCPClient.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <iphlpapi.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
constexpr auto DEFAULT_BUFLEN = 512;
TCPClient::TCPClient():
    m_ConnectSocket(INVALID_SOCKET)
{
    //Need to call this before making any winsocket api calls
    WSAData wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        throw std::exception("WSAStartup failed");
    }

    //Good to go
}

bool TCPClient::Connect(const std::string& ipAddress, const std::string& port)
{
    addrinfo* result, * ptr, hints;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo(ipAddress.c_str(), port.c_str(), &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed. %d\n", iResult);
        WSACleanup();
        return false;
    }

    //SOCKET connectSocket = INVALID_SOCKET;
    ptr = result;
    m_ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (m_ConnectSocket == INVALID_SOCKET)
    {
        printf("socket failed. %ld\n ", WSAGetLastError());
        freeaddrinfo(result);
        return false;
    }

    iResult = connect(m_ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("connect failed. %ld\n", WSAGetLastError());
        closesocket(m_ConnectSocket);
        m_ConnectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (m_ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return false;
    }
    return true;
}

void TCPClient::Send(const std::string& message)
{
}

void TCPClient::SetServerMessageDelegate(std::function<void(const std::string&)> delegator)
{
    processReceivedMessage = delegator;
}

void TCPClient::Close()
{
}
