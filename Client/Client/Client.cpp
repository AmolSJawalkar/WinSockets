// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "TCPClient.h"

void OnServerMessageReceived(const std::string& message)
{
    std::cout << message << std::endl;
}

int main()
{
    TCPClient client;
    client.SetServerMessageDelegate(OnServerMessageReceived);
    client.Connect("127.0.0.1", "27015");
    
    std::string message;
    do
    {
        message = "";
        std::getline(std::cin, message);
        client.Send(message);
    } while (message != "bye");


   return 0;
}
