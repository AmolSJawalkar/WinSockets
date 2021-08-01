#pragma once
#include "TCPServer.h"
class ServerProcess : private TCPServerEvents
{
	TCPServer _server;
	// Inherited via TCPServerEvents
	virtual void OnNewClientConnected(SOCKET clientId) override;
	virtual void OnClientDisconnected(SOCKET clientId) override;
	virtual void OnNewMessageFromClient(SOCKET clientId, const std::string& message) override;

public:
	void Start();
};

