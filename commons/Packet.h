#ifndef TWITTER_PACKET_H
#define TWITTER_PACKET_H

#include <string>

#define PACKET_BUFFER_LEN 4096

using namespace std;

enum CmdType
{
    PROFILE = 1,
    CLOSE_CONN = 2,
    FOLLOW = 3,
    SEND = 4
};

class Packet
{
public:
    Packet();
    Packet(string payload);
    Packet(CmdType cmd);
    Packet(CmdType cmd, string payload);
    char *toBytes();
    void fromBytes(char *buffer);
    CmdType getCmd();
    string getPayload();
    uint32_t getTimestamp();

private:
    CmdType cmd;
    string payload;
    uint32_t timestamp;
};

#endif
