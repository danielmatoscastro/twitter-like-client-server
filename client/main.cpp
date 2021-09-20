#include <iostream>
#include <string>
#include <signal.h>
#include <pthread.h>
#include "Connection.h"
#include "../commons/Packet.h"

#define MAX_MSG_LEN 128

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

    while (!cin.eof())
    {
        getline(cin, line);
        if (!con->isClosed())
        {
            Packet *packet;
            if (line.rfind("FOLLOW") == 0)
            {
                packet = new Packet(CmdType::FOLLOW, line.substr(7));
            }
            else if (line.rfind("SEND") == 0)
            {
                string message = line.substr(5);
                if (message.size() > MAX_MSG_LEN)
                {
                    cout << "Too long... max 128 chars." << endl;
                    continue;
                }
                packet = new Packet(CmdType::SEND, line.substr(5));
            }
            else
            {
                cout << "Sorry, wrong command..." << endl;
                continue;
            }

            con->sendPacket(packet);
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
        if (packet->getCmd() == CmdType::CLOSE_CONN)
        {
            con->close();
            cout << "Server sent CLOSE_CONN" << endl;
            exit(EXIT_SUCCESS);
        }
        else
        {
            cout << packet->getPayload() << endl;
        }
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
        cout << "too few arguments" << endl;
        exit(EXIT_FAILURE);
    }

    char *profile = argv[1];
    char *addr = argv[2];
    char *port = argv[3];

    if (strlen(profile) < 5 || strlen(profile) > 21)
    {
        cout << "profile must have [4-20] chars." << endl;
        exit(EXIT_FAILURE);
    }

    con = new Connection(stoi(port), addr);

    signal(SIGINT, interruption_handler);

    send_presentation(profile);

    pthread_create(&to_server_th, NULL, to_server, NULL);
    pthread_create(&from_server_th, NULL, from_server, NULL);

    pthread_join(to_server_th, NULL);
    pthread_join(from_server_th, NULL);

    return EXIT_SUCCESS;
}
