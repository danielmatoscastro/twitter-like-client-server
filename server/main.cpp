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
#include "../commons/Packet.h"
#include "../commons/Server.h"
#include "../commons/Connection.h"
#include "../commons/ClientConnection.h"
#include "Profile.h"
#include "ProfileAccessController.h"

#define PORT 4242

using namespace std;

Connection *routerConn;
ProfileAccessController *profiles;
Server *server;

Profile *receiveProfileCmd(ClientConnection *conn)
{
    Profile *profile;
    Packet *toClient;
    Packet *packet = conn->receivePacket();
    if (packet->getCmd() != CmdType::PROFILE)
    {
        toClient = new Packet(CmdType::CLOSE_CONN);
        conn->sendPacket(toClient);
        conn->close();
        pthread_exit(nullptr);
    }
    profile = profiles->createProfileIfNotExists(packet->getPayload(), conn);
    return profile;
}

void *from_client(void *_conn)
{
    ClientConnection *conn = (ClientConnection *)_conn;
    Profile *profile = receiveProfileCmd(conn);

    Packet *hello = new Packet(CmdType::SEND, "Hello client! " + profile->getProfileId());
    conn->sendPacket(hello);
    profile->fetchInboxContent();

    cout << "Client " << profile->getProfileId() << " connected." << endl
         << "Waiting for client message ..." << endl;

    bool clientWantsToQuit = false;
    while (!clientWantsToQuit && !conn->isClosed())
    {
        Packet *packet = conn->receivePacket();

        switch (packet->getCmd())
        {
        case CmdType::FOLLOW:
        {
            profiles->addFollowerTo(packet->getPayload(), profile);
            break;
        }
        case CmdType::CLOSE_CONN:
        {
            profile->decSessionsOn(conn);
            clientWantsToQuit = true;
            break;
        }
        case CmdType::SEND:
        {
            profiles->sendToFollowersOf(profile, packet);
            break;
        }
        default:
        {
            cout << "I dont know..." << endl;
            break;
        }
        }

        Packet *yep = new Packet(CmdType::SEND, "Yep!", "server");
        conn->sendPacket(yep);
    }

    conn->close();
    pthread_exit(nullptr);
}

void interruption_handler(sig_atomic_t sigAtomic)
{
    profiles->sendCloseConnToAll();
    server->close();
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, interruption_handler);
    routerConn = new Connection(3000, "127.0.0.1");
    routerConn->sendPacket(new Packet(CmdType::SET_PRIMARY_IF_NOT_EXISTS, "127.0.0.1:4242"));
    routerConn->sendPacket(new Packet(CmdType::CLOSE_CONN));
    routerConn->close();

    profiles = new ProfileAccessController("state.json");
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
