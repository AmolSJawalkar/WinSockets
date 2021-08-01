#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <iphlpapi.h>
#include <iostream>
#include "TCPServer.h"
#include "TCPClients.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

void TCPServer::Run()
{
    while (true)
    {
        FD_ZERO(&_fr);
        FD_ZERO(&_fw);
        FD_ZERO(&_fe);

        FD_SET(_listeningSocket, &_fr);
        FD_SET(_listeningSocket, &_fe);
        for (int index = 0; index < MAX_CLIENTS; index++)
        {
            if (_clients[index] != 0)
            {
                FD_SET(_clients[index], &_fr);
                FD_SET(_clients[index], &_fe);
            }
        }

        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int nRet = select(_listeningSocket + 1, &_fr, &_fw, &_fe, &tv);
        if (nRet > 0)
        {
            processNewConnection(_listeningSocket);
        }
        else if (nRet == 0)
        {
            //No updates on port
        }
        else
        {
            std::cout << "Select failed" << std::endl;
        }
    }
}


void TCPServer::procesNewMessage(SOCKET clientSocket)
{
    char buff[4096];
    auto bytesRecieved = recv(clientSocket, buff, 4096, 0);
    if (bytesRecieved < 0)
    {
        //Client disconnectd
        _clients.remove(clientSocket);
        if (_eventshandler != nullptr)
            _eventshandler->OnClientDisconnected(clientSocket);
    }
    else
    {
        if (_eventshandler != nullptr)
            _eventshandler->OnNewMessageFromClient(clientSocket, std::string(buff, bytesRecieved));
    }
}

void TCPServer::processNewConnection(SOCKET listeningSocket)
{
    //Check for read connections
    if (FD_ISSET(listeningSocket, &_fr))
    {
        sockaddr clientDetails = {};
        int addlen = 0;
        SOCKET clientSocket = accept(listeningSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET)
        {
            printf("Client Accept failed. %ld", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return;
        }

        _clients.add(clientSocket);
        if (_eventshandler != nullptr)
            _eventshandler->OnNewClientConnected(clientSocket);
    }
    else
    {
        for (int index = 0; index < MAX_CLIENTS; index++)
        {
            if (FD_ISSET(_clients[index], &_fr))
            {
                procesNewMessage(_clients[index]);
            }
        }

    }
}

TCPServer::TCPServer():
    _listeningSocket(INVALID_SOCKET),
    _eventshandler(nullptr)
{
	WSAData wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		throw std::exception("WSAStartup failed");
	}

	//got to go
}

bool TCPServer::SendMessageTo(SOCKET client, const std::string& message)
{
    std::string response = std::to_string(client) + " : " + message;
    auto sendResult = send(client, response.c_str(), response.size() + 1, 0);
    if (sendResult == SOCKET_ERROR)
    {
        return false;
    }
    return true;
}

TCPServer::~TCPServer()
{
    this->Close();
    _eventshandler = nullptr;
}

TCPClients& TCPServer::Clients()
{
    return _clients;
}

bool TCPServer::start(const std::string& port)
{
    addrinfo* result = nullptr, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    //2. Address Info
    auto iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed. %d\n", iResult);
        WSACleanup();
        return false;
    }

    //3. Create Socket
    //SOCKET listeningSocket = INVALID_SOCKET;
    _listeningSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (_listeningSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    //Blocking vs Non-blocking
    //u_long options = 0; //0 - blocking, 1-  non-blocking
    //iResult = ioctlsocket(ListenSocket, FIONBIO, &options);
    //if (iResult != 0)
    //{

    //}

    //4. Bind the socket to network address
    iResult = bind(_listeningSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed. %d", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(_listeningSocket);
        WSACleanup();
        return false;
    }

    freeaddrinfo(result);

    //5. Listening on socket
    if (listen(_listeningSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("listen failed. %d", WSAGetLastError());
        closesocket(_listeningSocket);
        WSACleanup();
        return false;
    }

    printf("Server started..Waiting for client connection");
	return true;
}

void TCPServer::Close()
{
    WSACleanup();
}

void TCPServer::SetEventsHanlder(TCPServerEvents* handler)
{
    _eventshandler = handler;
}
