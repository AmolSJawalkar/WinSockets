#pragma once
#include <array>
#include <WinSock2.h>
#define MAX_CLIENTS 5
class TCPClients
{
	std::array<SOCKET, MAX_CLIENTS> _clients;
public:
	TCPClients();
	~TCPClients();
	SOCKET& operator[](unsigned int index);
	std::array< SOCKET, MAX_CLIENTS> GetClients() const;
	bool add(SOCKET newSocket);
	void remove(SOCKET oldSocket);
};

