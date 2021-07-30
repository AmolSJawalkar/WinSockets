// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

constexpr auto DEFAULT_PORT = "27015";
constexpr auto DEFAULT_BUFLEN = 512;
int main()
{
   //Need to call this before making any winsocket api calls
   WSAData wsaData;
   int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (iResult != 0)
   {
      printf("WSAStartup failed. %d\n", iResult);
   }

   addrinfo* result, *ptr, hints;
   ZeroMemory(&hints, sizeof(hints));

   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;

   iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
   if (iResult != 0)
   {
      printf("getaddrinfo failed. %d\n", iResult);
      WSACleanup();
      return 1;
   }

   SOCKET connectSocket = INVALID_SOCKET;
   ptr = result;
   connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
   if (connectSocket == INVALID_SOCKET)
   {
      printf("socket failed. %ld\n ", WSAGetLastError());
      freeaddrinfo(result);
      return 1;
   }

   iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
   if (iResult == SOCKET_ERROR)
   {
      printf("connect failed. %ld\n", WSAGetLastError());
      closesocket(connectSocket);
      connectSocket = INVALID_SOCKET;
   }

   freeaddrinfo(result);

   if (connectSocket == INVALID_SOCKET)
   {
      printf("Unable to connect to server!\n");
      WSACleanup();
      return 1;
   }

   //Receive data until server closes the connection
   do 
   {
      int recvbuflen = DEFAULT_BUFLEN;
      std::string message;
      std::cout << "Me : ";
      std::getline(std::cin, message);
      std::cout << "\n";

      const char *sendbuf = message.c_str();

      iResult = send(connectSocket, sendbuf, (int)strlen(sendbuf), 0);
      if (iResult == SOCKET_ERROR)
      {
         printf("send failed. %ld", WSAGetLastError());
         closesocket(connectSocket);
         WSACleanup();
         return 1;
      }

      char recvbuf[DEFAULT_BUFLEN];
      iResult = recv(connectSocket, recvbuf, recvbuflen, 0);
      if (iResult > 0)
      {
         std::cout << "Server : " << std::string(recvbuf, iResult) <<std::endl;
      }
      else if (iResult == 0)
         printf("Connection closed\n");
      else
         printf("recv failed: %d\n", WSAGetLastError());
   } while (iResult > 0);


   // shutdown the send half of the connection since no more data will be sent
   iResult = shutdown(connectSocket, SD_SEND);
   if (iResult == SOCKET_ERROR) {
      printf("shutdown failed: %d\n", WSAGetLastError());
      closesocket(connectSocket);
      WSACleanup();
      return 1;
   }

   // cleanup
   closesocket(connectSocket);
   WSACleanup();

   return 0;
}
