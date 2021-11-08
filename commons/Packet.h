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
    SEND = 4,
    SET_PRIMARY_IF_NOT_EXISTS = 5,
    SET_PRIMARY = 6,
    GET_PRIMARY = 7,
    SET_BACKUP = 8,
    ALIVE = 9,
    OK = 10,
    BACKUP_PROPAGATION = 11
};

class Packet
{
public:
    Packet();
    Packet(string payload);
    Packet(CmdType cmd);
    Packet(CmdType cmd, string payload);
    Packet(CmdType cmd, string payload, string sender);
    char *toBytes();
    void fromBytes(char *buffer);
    CmdType getCmd();
    string getPayload();
    string getSender();
    time_t getTimestamp();

private:
    CmdType cmd;
    string payload;
    time_t timestamp;
    string sender;
};

#endif
