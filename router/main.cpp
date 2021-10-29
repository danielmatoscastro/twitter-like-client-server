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

void setPrimary(Packet *packet);

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
                setPrimary(packet);
                Packet *payload = new Packet(CmdType::OK, primaryServer);
                conn->sendPacket(payload);
                hasPrimary = true;
            }
            else
            {
                Packet *payload = new Packet(CmdType::SET_PRIMARY, primaryServer);
                conn->sendPacket(payload);
                cout << "Else do primary if not exists" << endl;
                //primario é fulano
                //novo servidor se conecta e vira backup
                //primário de tempos em tempos manda um recado dizendo que ta vivo
            }

            break;
        }
        case CmdType::SET_PRIMARY:
        {
            setPrimary(packet);
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
    }

    conn->close();
    pthread_exit(nullptr);
}

void setPrimary(Packet *packet)
{
    primaryServer = packet->getPayload();
    cout << "Primary server " << primaryServer << endl;
}

// void setPrimary(Packet* packet){
//     string payload = packet->getPayload();
//     // formato "addr:port"
//     size_t pos = payload.find(':');
//     string strAddr = payload.substr(0, pos);
//     string strPort = payload.substr(pos + 1);

//     uint16_t port = stoi(strPort);

//     primaryServer.sin_family = AF_INET;
//     primaryServer.sin_port = htons(port);
//     primaryServer.sin_addr.s_addr = inet_addr(strAddr.c_str());

//     memset(primaryServer.sin_zero, 0, 8);

//     cout << "Primary server " << "addr: " << strAddr << " port: " << strPort << endl;
// }

int main()
{
    signal(SIGINT, interruptionHandler);

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
