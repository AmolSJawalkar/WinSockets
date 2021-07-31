#pragma once
#include<string>
#include<functional>


class TCPClient
{
	std::function<void(const std::string&)> processReceivedMessage;
	unsigned __int64 m_ConnectSocket; //SOCKET
public:
	TCPClient();
	bool Connect(const std::string& ipAddress, const std::string& port);
	void Send(const std::string& message);
	void SetServerMessageDelegate(std::function<void(const std::string&)> delegator);
	void Close();
};

