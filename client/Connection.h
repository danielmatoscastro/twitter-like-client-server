//
// Created by daniel on 26/08/2021.
//
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

#include "../commons/Packet.h"

#ifndef TWITTER_CONNECTION_H
#define TWITTER_CONNECTION_H

using namespace std;

class Connection
{
public:
    Connection(uint16_t port, const char *server_addr);
    void close();
    bool isClosed();
    void sendPacket(Packet *packet);
    Packet *receivePacket();

private:
    struct sockaddr_in server;
    int sockfd;
    int len = sizeof(server);
    char buffer_in[PACKET_BUFFER_LEN];
    char buffer_out[PACKET_BUFFER_LEN];
    bool closed;
    void sendMessage(const char *msg);
    char *receiveMessage();
};

#endif //TWITTER_CONNECTION_H
