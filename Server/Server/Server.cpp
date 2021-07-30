// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#define MAX_CLIENTS 5

fd_set fr, fw, fe;

SOCKET ClientSockets[MAX_CLIENTS];

bool addToClients(SOCKET newSocket)
{
   for (int index = 0; index < MAX_CLIENTS; index++)
   {
      if (ClientSockets[index] == 0)
      {
         ClientSockets[index] = newSocket;
         return true;
      }
   }

   return false;
}

void removeFromClients(SOCKET oldSocket)
{
   for (int index = 0; index < MAX_CLIENTS; index++)
   {
      if (ClientSockets[index] == oldSocket)
      {
         ClientSockets[index] = 0;
         break;
      }
   }
}

void sendWelcomeMessage(SOCKET client, sockaddr details)
{
   std::string message = "Welcome. You got connected to server.";
   auto sendResult = send(client, message.c_str(), message.size() + 1, 0);
   if (sendResult == SOCKET_ERROR)
   {
      
   }
}

void ProcesNewMessage(SOCKET clientSocket)
{
   char buff[4096];
   auto bytesRecieved = recv(clientSocket, buff, 4096, 0);
   if (bytesRecieved < 0)
   {
      //Client disconnectd
      removeFromClients(clientSocket);
      std::cout << std::endl << "Closed the connection for Client " << clientSocket;
   }
   else
   {
      std::cout << std::endl << " Client " << clientSocket << " : " << std::string(buff, bytesRecieved);
   }

}

void ProcessNewConnection(SOCKET listeningSocket)
{
   //Check for read connections
   if (FD_ISSET(listeningSocket, &fr))
   {
      sockaddr clientDetails = {};
      int addlen = 0;
      SOCKET clientSocket = accept(listeningSocket, nullptr, nullptr);
      if (clientSocket == INVALID_SOCKET)
      {
         printf("Client Accept failed. %ld", WSAGetLastError());
         closesocket(clientSocket);
         WSACleanup();
         return;
      }

      addToClients(clientSocket);
      std::cout << std::endl << "New Client connect. ID : " << clientSocket;;
      sendWelcomeMessage(clientSocket, clientDetails);
   }
   else
   {
      for (int index = 0; index < MAX_CLIENTS; index++)
      {
         if (FD_ISSET(ClientSockets[index], &fr))
         {
            ProcesNewMessage(ClientSockets[index]);
         }
      }

   }

   if (FD_ISSET(listeningSocket, &fw))
   {
      std::cout << std::endl << "Something on port to send";
   }


   if (FD_ISSET(listeningSocket, &fe))
   {
      std::cout << std::endl << "An error on port";
   }
}


int main()
{
   //1. Start
   WSAData wsaData;
   int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (iResult != 0)
   {
      printf("WSAStartup failed. %d\n", iResult);
      return 0;
   }

   addrinfo* result = nullptr, hints;
   ZeroMemory(&hints, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags = AI_PASSIVE;

   //2. Address Info
   iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
   if (iResult != 0)
   {
      printf("getaddrinfo failed. %d\n", iResult);
      WSACleanup();
      return 1;
   }

   //3. Create Socket
   SOCKET listeningSocket = INVALID_SOCKET;
   listeningSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
   if (listeningSocket == INVALID_SOCKET)
   {
      printf("Error at socket(): %d\n", WSAGetLastError());
      freeaddrinfo(result);
      WSACleanup();
      return 1;
   }

   //Blocking vs Non-blocking
   //u_long options = 0; //0 - blocking, 1-  non-blocking
   //iResult = ioctlsocket(ListenSocket, FIONBIO, &options);
   //if (iResult != 0)
   //{

   //}

   //4. Bind the socket to network address
   iResult = bind(listeningSocket, result->ai_addr, (int)result->ai_addrlen);
   if (iResult == SOCKET_ERROR)
   {
      printf("bind failed. %d", WSAGetLastError());
      freeaddrinfo(result);
      closesocket(listeningSocket);
      WSACleanup();
      return 1;
   }

   freeaddrinfo(result);

   //5. Listening on socket
   if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR)
   {
      printf("listen failed. %d", WSAGetLastError());
      closesocket(listeningSocket);
      WSACleanup();
      return 1;
   }

   printf("Server started..Waiting for client connection");

   while (true)
   {
      FD_ZERO(&fr);
      FD_ZERO(&fw);
      FD_ZERO(&fe);

      FD_SET(listeningSocket, &fr);
      FD_SET(listeningSocket, &fe);
      for (int index = 0; index < MAX_CLIENTS; index++)
      {
         if (ClientSockets[index] != 0)
         {
           FD_SET(ClientSockets[index], &fr);
           FD_SET(ClientSockets[index], &fe);
         }
      }

      timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;

      int nRet = select(listeningSocket + 1, &fr, &fw, &fe, &tv);
      if (nRet > 0)
      {
         ProcessNewConnection(listeningSocket);
      }
      else if (nRet == 0)
      {
         //No updates on port
      }
      else
      {

      }
   }

   /*
   //6. Accept connection
   SOCKET clientSocket;
   clientSocket = accept(ListenSocket, NULL, NULL);
   if (clientSocket == INVALID_SOCKET)
   {
      printf("accept failed. %d", WSAGetLastError());
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
   }

   //No longer need server socket
   closesocket(ListenSocket);

   //7. Receive and send data
   char recbuf[DEFAULT_BUFLEN];
   int iSendResult;
   int recbuflen = DEFAULT_BUFLEN;
   //Receive until client shuts down
   std::cout << std::endl;
   do
   {
      ZeroMemory(recbuf, DEFAULT_BUFLEN);
      iResult = recv(clientSocket, recbuf, recbuflen, 0);
      if (iResult > 0)
      {
         //printf("Bytes Received : %d\n", iResult);
         std::string message = std::string(recbuf, iResult);
         std::cout << "Client : " << message << std::endl;

         // Echo the buffer back to client
         std::string response;
         std::cout << "Me : ";
         std::getline(std::cin, response);
         std::cout << "\n";
         const char* sendbuf = response.c_str();
         iSendResult = send(clientSocket, sendbuf, strlen(sendbuf), 0);
         if (iSendResult == SOCKET_ERROR)
         {
            printf("sent failed. %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
         }

         //printf("Bytes sent : %d\n", iSendResult);
      }
      else if (iResult == 0)
      {
         printf("connection closing...\n");
      }
      else
      {
         printf("recv failed: %d\n", WSAGetLastError());
         closesocket(clientSocket);
         WSACleanup();
         return 1;
      }
   } while (iResult > 0); //iResult stores no of bytes send or receive



   iResult = shutdown(clientSocket, SD_SEND);
   if (iResult == SOCKET_ERROR)
   {
      printf("shutdown failed. %d\n", WSAGetLastError());
      closesocket(clientSocket);
      WSACleanup();
      return 1;;
   }

   closesocket(clientSocket);
      */
   WSACleanup();

   return 0;
}

