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

enum CmdType
{
    PROFILE = 1,
    CLOSE_CONN = 2
};

class Packet
{
public:
    Packet();
    Packet(PacketType type, uint16_t sequence, string payload);
    Packet(uint16_t sequence, string payload);
    Packet(CmdType cmd, uint16_t sequence);
    Packet(CmdType cmd, uint16_t sequence, string payload);
    char *toBytes();
    void fromBytes(char *buffer);
    PacketType getType();
    CmdType getCmd();
    uint16_t getSequence();
    string getPayload();
    uint32_t getTimestamp();

private:
    PacketType type;
    CmdType cmd;
    uint16_t sequence;
    string payload;
    uint32_t timestamp;
};

#endif
