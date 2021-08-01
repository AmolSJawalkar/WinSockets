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
#include <future>

#pragma comment(lib, "Ws2_32.lib")
constexpr auto DEFAULT_BUFLEN = 512;

TCPClient::TCPClient() :
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

TCPClient::~TCPClient()
{
	this->Close();
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

	std::thread sendThrd(&TCPClient::sendThread, this);
	std::thread recveThrd(&TCPClient::receievedThread, this);
	sendThrd.detach();
	recveThrd.detach();
	return true;
}

void TCPClient::Send(const std::string& message)
{
	_sendQueue.Enqueue(message);
}

void TCPClient::SetServerMessageDelegate(std::function<void(const std::string&)> delegator)
{
	processReceivedMessage = delegator;
}

void TCPClient::Close()
{
	// shutdown the send half of the connection since no more data will be sent
	int iResult = shutdown(m_ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(m_ConnectSocket);
		WSACleanup();
		return;
	}

	// cleanup
	closesocket(m_ConnectSocket);
	WSACleanup();
}

void TCPClient::sendThread()
{
	fd_set fw, fe;
	while (true)
	{
		auto message = _sendQueue.Dequeue();

		FD_ZERO(&fw);
		FD_ZERO(&fe);

		FD_SET(m_ConnectSocket, &fw);
		FD_SET(m_ConnectSocket, &fe);

		timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int nRet = select(m_ConnectSocket + 1, nullptr, &fw, &fe, &tv);
		if (nRet > 0)
		{
			if (FD_ISSET(m_ConnectSocket, &fw))
			{
				int iResult = send(m_ConnectSocket, message.c_str(), message.size() + 1, 0);
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed. %ld", WSAGetLastError());
					closesocket(m_ConnectSocket);
					WSACleanup();
					return;
				}
			}
			else
			{
				std::cout << "Send to fail message. " << message;
			}
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

void TCPClient::receievedThread()
{
	fd_set fr, fe;
	while (true)
	{
		FD_ZERO(&fr);
		FD_ZERO(&fe);

		FD_SET(m_ConnectSocket, &fr);
		FD_SET(m_ConnectSocket, &fe);

		timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int nRet = select(m_ConnectSocket + 1, &fr, nullptr, &fe, &tv);
		if (nRet > 0)
		{
			if (FD_ISSET(m_ConnectSocket, &fr))
			{
				char buff[4096];
				auto bytesRecieved = recv(m_ConnectSocket, buff, 4096, 0);
				if (bytesRecieved < 0)
				{
					//Client disconnectd
					std::cout << std::endl << "Closed the connection for Server ";
				}
				else
				{
					if (processReceivedMessage != nullptr)
						processReceivedMessage(std::string(buff, bytesRecieved));
				}
			}
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
