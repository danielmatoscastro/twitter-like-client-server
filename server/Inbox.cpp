#include "Inbox.h"

using namespace std;

Inbox::Inbox()
{
    this->inbox = new queue<Packet *>();
}

void Inbox::insertPacket(Packet *packet)
{
    this->inbox->push(packet);
}

Packet *Inbox::popPacket()
{
    Packet *packet = this->inbox->front();
    this->inbox->pop();
    return packet;
}

bool Inbox::hasPacket()
{
    return !this->inbox->empty();
}
