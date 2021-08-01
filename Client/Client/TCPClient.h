#pragma once
#include<string>
#include<functional>
#include <queue>
#include "ProducerConsumerQueue.h"

class TCPClient
{
	std::function<void(const std::string&)> processReceivedMessage;
	unsigned __int64 m_ConnectSocket; //SOCKET
	ProducerConsumerQueue<std::string> _sendQueue;
	void sendThread();
	void receievedThread();
public:
	TCPClient();
	bool Connect(const std::string& ipAddress, const std::string& port);
	void Send(const std::string& message);
	void SetServerMessageDelegate(std::function<void(const std::string&)> delegator);
	void Close();
};

