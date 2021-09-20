#ifndef TWITTER_INBOX_H
#define TWITTER_INBOX_H

#include <string>
#include <queue>
#include "../commons/Packet.h"

using namespace std;

class Inbox
{
public:
    Inbox();
    void insertPacket(Packet *packet);
    Packet *popPacket();
    bool hasPacket();

private:
    queue<Packet *> *inbox;
};

#endif
