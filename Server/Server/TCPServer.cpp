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
            ProcessNewConnection(_listeningSocket);
        }
        else if (nRet == 0)
        {
            //No updates on port
        }
        else
        {

        }
    }
}

bool TCPServer::addToClients(SOCKET newSocket)
{
    for (int index = 0; index < MAX_CLIENTS; index++)
    {
        if (_clients[index] == 0)
        {
            _clients[index] = newSocket;
            return true;
        }
    }

    return false;
}

void TCPServer::removeFromClients(SOCKET oldSocket)
{
    for (int index = 0; index < MAX_CLIENTS; index++)
    {
        if (_clients[index] == oldSocket)
        {
            _clients[index] = 0;
            break;
        }
    }
}

void TCPServer::sendWelcomeMessage(SOCKET client, sockaddr details)
{
    std::string message = "Welcome. You got connected to server.";
    auto sendResult = send(client, message.c_str(), message.size() + 1, 0);
    if (sendResult == SOCKET_ERROR)
    {

    }
}

void TCPServer::broadcastMessage(SOCKET from, const std::string& message)
{
    for (int index = 0; index < MAX_CLIENTS; index++)
    {
        auto clientID = _clients[index];
        if (clientID != 0 && clientID != from)
        {
            std::string response = std::to_string(clientID) + " : " + message;
            auto sendResult = send(clientID, response.c_str(), response.size() + 1, 0);
            if (sendResult == SOCKET_ERROR)
            {

            }
        }
    }
}

void TCPServer::ProcesNewMessage(SOCKET clientSocket)
{
    char buff[4096];
    auto bytesRecieved = recv(clientSocket, buff, 4096, 0);
    if (bytesRecieved < 0)
    {
        //Client disconnectd
        removeFromClients(clientSocket);
        std::cout << std::endl << "Closed the connection for Client " << clientSocket;
    }
    else
    {
        std::string message(buff, bytesRecieved);
        std::cout << std::endl << "Client " << clientSocket << " : " << message;
        broadcastMessage(clientSocket, message);
    }

}

void TCPServer::ProcessNewConnection(SOCKET listeningSocket)
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

        addToClients(clientSocket);
        std::cout << std::endl << "New Client connect. ID : " << clientSocket;;
        sendWelcomeMessage(clientSocket, clientDetails);
    }
    else
    {
        for (int index = 0; index < MAX_CLIENTS; index++)
        {
            if (FD_ISSET(_clients[index], &_fr))
            {
                ProcesNewMessage(_clients[index]);
            }
        }

    }
}

TCPServer::TCPServer():
    _listeningSocket(INVALID_SOCKET)
{
	WSAData wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		throw std::exception("WSAStartup failed");
	}

	//got to go
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        _clients[i] = 0;
    }
}

TCPServer::~TCPServer()
{
    this->Close();
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
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        _clients[i] = INVALID_SOCKET;
    }
}
