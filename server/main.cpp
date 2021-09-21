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
#include <iostream>
#include "../commons/Packet.h"
#include "Server.h"
#include "ClientConnection.h"
#include "Profile.h"
#include "ProfilesManager.h"

#define PORT 4242

using namespace std;

ProfilesManager *profiles;

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
    cout << "pre-hi" << endl;
    profile = profiles->createProfileIfNotExists(packet->getPayload(), conn);
    cout << "hi" << endl;
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
    while (!clientWantsToQuit)
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

int main()
{
    profiles = new ProfilesManager("state.json");
    Server *server = new Server(PORT);

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
