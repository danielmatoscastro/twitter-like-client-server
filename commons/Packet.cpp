#include <cstring>
#include <string>
#include <ctime>
#include <iostream>

using namespace std;

#include "Packet.h"

Packet::Packet()
{
}

Packet::Packet(string payload)
{
    this->cmd = CmdType::SEND;
    this->payload = payload;
    this->timestamp = time(nullptr);
    this->sender = "";
}

Packet::Packet(CmdType cmd)
{
    this->cmd = cmd;
    this->payload = "";
    this->timestamp = time(nullptr);
    this->sender = "";
}

Packet::Packet(CmdType cmd, string payload)
{
    this->cmd = cmd;
    this->payload = payload;
    this->timestamp = time(nullptr);
    this->sender = "";
}

Packet::Packet(CmdType cmd, string payload, string sender)
{
    this->cmd = cmd;
    this->payload = payload;
    this->timestamp = time(nullptr);
    this->sender = sender;
}

char *Packet::toBytes()
{
    char *buffer = new char[PACKET_BUFFER_LEN];
    memset(buffer, 0, PACKET_BUFFER_LEN);
    int pos = 0;

    buffer[pos] = (char)this->cmd;
    pos += sizeof(char);

    memcpy(&buffer[pos], &(this->timestamp), sizeof(this->timestamp));
    pos += sizeof(this->timestamp);

    size_t payload_size = this->payload.size();
    memcpy(&buffer[pos], &payload_size, sizeof(payload_size));
    pos += sizeof(payload_size);

    const char *payload_char = this->payload.c_str();
    memcpy(&buffer[pos], payload_char, payload_size);
    pos += payload_size;

    size_t sender_size = this->sender.size();
    memcpy(&buffer[pos], &sender_size, sizeof(sender_size));
    pos += sizeof(sender_size);

    const char *sender_char = this->sender.c_str();
    memcpy(&buffer[pos], sender_char, sender_size);
    pos += sender_size;

    return buffer;
}

void Packet::fromBytes(char *buffer)
{
    int pos = 0;

    this->cmd = (CmdType)buffer[pos];
    pos += sizeof(char);
    //cout << this->cmd << endl;

    memcpy(&(this->timestamp), &buffer[pos], sizeof(this->timestamp));
    pos += sizeof(this->timestamp);
    //cout << this->timestamp << endl;

    size_t payload_size;
    memcpy(&payload_size, &buffer[pos], sizeof(size_t));
    pos += sizeof(payload_size);
    //cout << payload_size << endl;

    char *payload_char = new char[payload_size + 1];
    memset(payload_char, 0, payload_size + 1);
    memcpy(payload_char, &buffer[pos], payload_size);
    this->payload = string(payload_char);
    pos+=payload_size;
    //cout << this->payload << endl;

    size_t sender_size;
    memcpy(&sender_size, &buffer[pos], sizeof(size_t));
    pos += sizeof(sender_size);

    char *sender_char = new char[sender_size + 1];
    memset(sender_char, 0, sender_size + 1);
    memcpy(sender_char, &buffer[pos], sender_size);
    this->sender = string(sender_char);
    //cout << this->sender << endl;
}

CmdType Packet::getCmd()
{
    return this->cmd;
}

string Packet::getPayload()
{
    return this->payload;
}

string Packet::getSender()
{
    return this->sender;
}

time_t Packet::getTimestamp()
{
    return this->timestamp;
}
