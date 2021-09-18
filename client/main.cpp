#include <iostream>
#include <string>
#include <signal.h>
#include <pthread.h>
#include "Connection.h"
#include "../commons/Packet.h"

Connection *con;
pthread_t to_server_th;
pthread_t from_server_th;

void interruption_handler(sig_atomic_t sigAtomic)
{
    Packet *packet = new Packet(CmdType::CLOSE_CONN);
    con->sendPacket(packet);
    con->close();
    exit(EXIT_SUCCESS);
}

void *to_server(void *args)
{
    string line;
    getline(cin, line);
    while (!cin.eof())
    {
        if (!con->isClosed())
        {
            Packet *packet = new Packet(line);
            con->sendPacket(packet);
            getline(cin, line);
        }
        else
        {
            pthread_exit(NULL);
        }
    }

    con->close();
    pthread_exit(NULL);
}

void *from_server(void *args)
{
    while (!con->isClosed())
    {
        Packet *packet = con->receivePacket();
        cout << packet->getPayload() << endl;
    }

    pthread_exit(NULL);
}

void send_presentation(char *profile)
{
    Packet *packet = new Packet(CmdType::PROFILE, profile);
    con->sendPacket(packet);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        perror("too few arguments\n");
        exit(EXIT_FAILURE);
    }

    char *profile = argv[1];
    char *addr = argv[2];
    char *port = argv[3];
    con = new Connection(stoi(port), addr);

    signal(SIGINT, interruption_handler);

    send_presentation(profile);

    pthread_create(&to_server_th, NULL, to_server, NULL);
    pthread_create(&from_server_th, NULL, from_server, NULL);

    pthread_join(to_server_th, NULL);
    pthread_join(from_server_th, NULL);

    return EXIT_SUCCESS;
}
