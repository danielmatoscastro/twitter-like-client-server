#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
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

void interruptionHandler(sig_atomic_t sigAtomic)
{
    exit(EXIT_SUCCESS);
}

Server *server;
string primaryServer;
bool hasPrimary = false;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void setPrimary(Packet *packet)
{
    primaryServer = packet->getPayload();
    cout << "Primary server: " << primaryServer << endl;
}

void *fromClient(void *_conn)
{
    ClientConnection *conn = (ClientConnection *)_conn;
    bool clientWantsToQuit = false;
    while (!clientWantsToQuit)
    {
        Packet *packet;
        try{
            packet = conn->receivePacket();
        }catch(...){
            pthread_exit(EXIT_SUCCESS);
        }
        
        switch (packet->getCmd())
        {
            case CmdType::SET_PRIMARY_IF_NOT_EXISTS:
            {
                pthread_mutex_lock(&m);
                if (!hasPrimary)
                {
                    setPrimary(packet);
                    Packet *payload = new Packet(CmdType::OK, primaryServer);
                    conn->sendPacket(payload);
                    hasPrimary = true;
                }
                else
                {
                    Packet *payload = new Packet(CmdType::SET_PRIMARY, primaryServer);
                    conn->sendPacket(payload);
                }
                pthread_mutex_unlock(&m);
                break;
            }
            case CmdType::SET_PRIMARY:
            {
                setPrimary(packet);
                Packet *payload = new Packet(CmdType::OK, primaryServer);
                conn->sendPacket(payload);
                break;
            }
            case CmdType::GET_PRIMARY:
            {
                Packet *payload = new Packet(CmdType::SET_PRIMARY, primaryServer);
                conn->sendPacket(payload);
                break;
            }
            case CmdType::CLOSE_CONN:
            {
                clientWantsToQuit = true;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    conn->close();
    pthread_exit(nullptr);
}

int main()
{
    signal(SIGINT, interruptionHandler);

    server = new Server(PORT);

    while (true)
    {
        ClientConnection *conn = server->waitClient();

        pthread_t *th = new pthread_t();
        if (pthread_create(th, NULL, fromClient, conn) != 0)
        {
            perror("pthread_create error:");
            return EXIT_FAILURE;
        }
    }

    server->close();

    cout << "Connection closed" << endl;

    return EXIT_SUCCESS;
}
