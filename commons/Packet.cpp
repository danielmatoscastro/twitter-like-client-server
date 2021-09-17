#include <cstring>
#include <string>
#include <ctime>

using namespace std;

#include "Packet.h"

Packet::Packet()
{
}

Packet::Packet(PacketType type, uint16_t sequence, string payload)
{
    this->type = type;
    this->sequence = sequence;
    this->payload = payload;
    this->timestamp = time(nullptr);
}

char *Packet::toBytes()
{
    char *buffer = new char[4096];
    int pos = 0;

    if (this->type == PacketType::DATA)
    {
        buffer[pos] = (char)1;
    }
    else
    {
        buffer[pos] = (char)2;
    }
    pos += 1;

    memcpy(&buffer[pos], &(this->sequence), sizeof(this->sequence));
    pos += sizeof(this->sequence);

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

    this->type = (PacketType)buffer[pos];
    pos += 1;

    memcpy(&(this->sequence), &buffer[pos], sizeof(this->sequence));
    pos += sizeof(this->sequence);

    memcpy(&(this->timestamp), &buffer[pos], sizeof(this->timestamp));
    pos += sizeof(this->timestamp);

    size_t payload_size;
    memcpy(&payload_size, &buffer[pos], sizeof(size_t));
    pos += sizeof(payload_size);

    char *payload_char = new char[payload_size + 1];
    memcpy(payload_char, &buffer[pos], payload_size);
    this->payload = string(payload_char);
}
