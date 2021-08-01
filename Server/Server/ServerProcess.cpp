#include "ServerProcess.h"
#include <iostream>

void ServerProcess::OnNewClientConnected(SOCKET clientId)
{
	_server.SendMessageTo(clientId, "Welcome. You got connected to server!");
}

void ServerProcess::OnClientDisconnected(SOCKET clientId)
{
	std::cout << std::endl << "Closed the connection for Client " << clientId;
}

void ServerProcess::OnNewMessageFromClient(SOCKET clientId, const std::string& message)
{
	std::cout << std::endl << "Client " << clientId << " : " << message;
	//Broadcase the message
	for (size_t i = 0; i < MAX_CLIENTS; i++)
	{
		if (_server.Clients()[i] != 0 && _server.Clients()[i] != clientId)
			_server.SendMessageTo(_server.Clients()[i], message);
	}
}

void ServerProcess::Start()
{
	_server.SetEventsHanlder(this);
	_server.start("27015");
	_server.Run();
}
