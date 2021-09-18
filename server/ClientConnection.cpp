#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ClientConnection.h"

using namespace std;

ClientConnection::ClientConnection(int clientfd)
{
    this->clientfd = clientfd;
}

void ClientConnection::sendMessage(const char *msg)
{
    char *buffer_out = new char[PACKET_BUFFER_LEN];
    memset(buffer_out, 0, PACKET_BUFFER_LEN);
    memcpy(buffer_out, msg, PACKET_BUFFER_LEN);

    send(this->clientfd, buffer_out, PACKET_BUFFER_LEN, 0);
}

char *ClientConnection::receiveMessage()
{
    char *buffer_in = new char[PACKET_BUFFER_LEN];
    memset(buffer_in, 0x0, PACKET_BUFFER_LEN);
    recv(this->clientfd, buffer_in, PACKET_BUFFER_LEN, 0);
    return buffer_in;
}

void ClientConnection::sendPacket(Packet *packet)
{
    char *buffer_temp = packet->toBytes();
    this->sendMessage(buffer_temp);
}

Packet *ClientConnection::receivePacket()
{
    char *buffer_temp = this->receiveMessage();
    Packet *packet = new Packet();
    packet->fromBytes(buffer_temp);
    return packet;
}

void ClientConnection::close()
{
    ::close(this->clientfd);
}
