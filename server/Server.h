#ifndef TWITTER_SERVER_H
#define TWITTER_SERVER_H

#include "ClientConnection.h"

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
