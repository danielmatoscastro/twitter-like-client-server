//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <iostream>
#include "../commons/Server.h"
#include "../commons/Packet.h"

#define PORT 3000

using namespace std;

void interruption_handler(sig_atomic_t sigAtomic)
{
    exit(EXIT_SUCCESS);
}

Server *server;

struct sockaddr_in primaryServer;
bool hasPrimary = false;

void *from_client(void *_conn)
{
    ClientConnection *conn = (ClientConnection *)_conn;
    bool clientWantsToQuit = false;
    while (!clientWantsToQuit)
    {
        Packet *packet = conn->receivePacket();
        switch (packet->getCmd())
        {
        case CmdType::SET_PRIMARY_IF_NOT_EXISTS:
        {
            if (!hasPrimary)
            {
                string payload = packet->getPayload();
                // formato "addr:port"
                size_t pos = payload.find(':');
                string addr = payload.substr(0, pos);
                string port = payload.substr(pos + 1);

                cout << "addr: " << addr << " port: " << port << endl;
            }

            break;
        }
        case CmdType::CLOSE_CONN:
        {
            cout << "client wants to quit" << endl;
            clientWantsToQuit = true;

            break;
        }
        default:
        {
            cout << "I dont know..." << endl;
            break;
        }
        }

        Packet *yep = new Packet(CmdType::SEND, "Yep!", "router");
        conn->sendPacket(yep);
    }

    conn->close();
    pthread_exit(nullptr);
}

int main()
{
    signal(SIGINT, interruption_handler);

    server = new Server(PORT);

    while (true)
    {
        ClientConnection *conn = server->waitClient();

        pthread_t *th = new pthread_t();
        if (pthread_create(th, NULL, from_client, conn) != 0)
        {
            perror("pthread_create error:");
            return EXIT_FAILURE;
        }
    }

    server->close();

    cout << "Connection closed" << endl;

    return EXIT_SUCCESS;
}
