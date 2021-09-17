#include <string>

#ifndef TWITTER_PACKET_H
#define TWITTER_PACKET_H

#define PACKET_BUFFER_LEN 4096
using namespace std;

enum PacketType
{
    DATA = 1,
    CMD = 2
};

class Packet
{
public:
    Packet();
    Packet(PacketType type, uint16_t sequence, string payload);
    char *toBytes();
    void fromBytes(char *buffer);
    PacketType getType();
    uint16_t getSequence();
    string getPayload();
    uint32_t getTimestamp();

private:
    PacketType type;
    uint16_t sequence;
    string payload;
    uint32_t timestamp;
};

#endif
