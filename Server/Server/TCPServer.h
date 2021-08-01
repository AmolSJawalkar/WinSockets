#pragma once
#include <string>
#include "TCPClients.h"
class TCPServerEvents
{
public:
	virtual void OnNewClientConnected(SOCKET clientId) = 0;
	virtual void OnClientDisconnected(SOCKET clientId) = 0;
	virtual void OnNewMessageFromClient(SOCKET clientId, const std::string& message) = 0;
};


class TCPServer
{
	TCPClients  _clients;
	SOCKET _listeningSocket;
	fd_set _fr, _fw, _fe;
	TCPServerEvents* _eventshandler;
	void procesNewMessage(SOCKET clientSocket);
	void processNewConnection(SOCKET listeningSocket);

public:
	bool SendMessageTo(SOCKET client, const std::string& message);
	TCPServer();
	~TCPServer();
	TCPClients& Clients();
	bool start(const std::string& port);
	void Run();
	void Close();
	void SetEventsHanlder(TCPServerEvents* handler);
};

