#include "../commons/Packet.h"

#ifndef TWITTER_CLIENT_CONNECTION_H
#define TWITTER_CLIENT_CONNECTION_H

using namespace std;

class ClientConnection
{
public:
    ClientConnection(int clientfd);
    void close();
    bool isClosed();
    void sendPacket(Packet *packet);
    Packet *receivePacket();

private:
    int clientfd;
    void send_message(const char *msg);
    char *receive_message();
};

#endif
