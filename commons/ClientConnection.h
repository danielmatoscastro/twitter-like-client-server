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
    bool closed;
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    void sendMessage(const char *msg);
    char *receiveMessage();
};

#endif
