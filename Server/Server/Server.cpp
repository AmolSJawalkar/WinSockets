// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string>
#include "TCPServer.h"

int main()
{
	TCPServer server;
	server.start("27015");
	server.Run();
   return 0;
}

