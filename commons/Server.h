#ifndef TWITTER_SERVER_H
#define TWITTER_SERVER_H

#define BACKLOG_SIZE 100

#include "../commons/ClientConnection.h"

using namespace std;

class Server
{
public:
    Server(uint16_t port);
    void close();
    ClientConnection *waitClient();

private:
    int serverfd;
};

#endif
