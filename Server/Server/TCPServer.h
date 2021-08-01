#pragma once
#include <string>
#include <WinSock2.h>

#define MAX_CLIENTS 5

class TCPServer
{
	SOCKET _clients[MAX_CLIENTS];
	SOCKET _listeningSocket;
	fd_set _fr, _fw, _fe;

	bool addToClients(SOCKET newSocket);
	void removeFromClients(SOCKET oldSocket);
	void sendWelcomeMessage(SOCKET client, sockaddr details);
	void broadcastMessage(SOCKET from, const std::string& message);
	void ProcesNewMessage(SOCKET clientSocket);
	void ProcessNewConnection(SOCKET listeningSocket);
public:
	TCPServer();
	~TCPServer();
	bool start(const std::string& port);
	void Run();
	void Close();
};

