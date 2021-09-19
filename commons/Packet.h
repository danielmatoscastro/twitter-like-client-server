#ifndef TWITTER_PACKET_H
#define TWITTER_PACKET_H

#include <string>

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
    CLOSE_CONN = 2,
    FOLLOW = 3
};

class Packet
{
public:
    Packet();
    Packet(PacketType type, string payload);
    Packet(string payload);
    Packet(CmdType cmd);
    Packet(CmdType cmd, string payload);
    char *toBytes();
    void fromBytes(char *buffer);
    PacketType getType();
    CmdType getCmd();
    string getPayload();
    uint32_t getTimestamp();

private:
    PacketType type;
    CmdType cmd;
    string payload;
    uint32_t timestamp;
};

#endif
