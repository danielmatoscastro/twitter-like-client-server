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
}

Packet::Packet(CmdType cmd)
{
    this->cmd = cmd;
    this->payload = "";
    this->timestamp = time(nullptr);
}

Packet::Packet(CmdType cmd, string payload)
{
    this->cmd = cmd;
    this->payload = payload;
    this->timestamp = time(nullptr);
}

char *Packet::toBytes()
{
    char *buffer = new char[PACKET_BUFFER_LEN];
    int pos = 0;

    buffer[pos] = (char)this->cmd;
    pos += sizeof(char);

    memcpy(&buffer[pos], &(this->timestamp), sizeof(this->timestamp));
    pos += sizeof(this->timestamp);

    size_t payload_size = this->payload.length();
    memcpy(&buffer[pos], &payload_size, sizeof(payload_size));
    pos += sizeof(payload_size);

    const char *payload_char = this->payload.c_str();
    memcpy(&buffer[pos], payload_char, payload_size);
    pos += payload_size;

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
    memcpy(payload_char, &buffer[pos], payload_size);
    this->payload = string(payload_char);
    //cout << this->payload << endl;
}

CmdType Packet::getCmd()
{
    return this->cmd;
}

string Packet::getPayload()
{
    return this->payload;
}
uint32_t Packet::getTimestamp()
{
    return this->timestamp;
}
