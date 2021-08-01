#include "TCPClients.h"

TCPClients::TCPClients()
{
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        _clients[i] = 0;
    }
}

TCPClients::~TCPClients()
{
    for (size_t i = 0; i < MAX_CLIENTS; i++)
    {
        _clients[i] = 0;
    }
}

SOCKET& TCPClients::operator[](unsigned int index)
{
    return _clients[index];
}

std::array<SOCKET, MAX_CLIENTS> TCPClients::GetClients() const
{
    return _clients;
}

bool TCPClients::add(SOCKET newSocket)
{
    for (int index = 0; index < MAX_CLIENTS; index++)
    {
        if (_clients[index] == 0)
        {
            _clients[index] = newSocket;
            return true;
        }
    }

    return false;
}

void TCPClients::remove(SOCKET oldSocket)
{
    for (int index = 0; index < MAX_CLIENTS; index++)
    {
        if (_clients[index] == oldSocket)
        {
            _clients[index] = 0;
            break;
        }
    }
}
