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
#define MAX_SESSIONS 2

using namespace std;

ProfilesManager *profiles;

Profile *receiveProfileCmd(ClientConnection *conn)
{
    Profile *profile;
    Packet *toClient;
    Packet *packet = conn->receivePacket();
    if (packet->getCmd() != CmdType::PROFILE)
    {
        cout << "Vai morrer again bro" << endl;
        toClient = new Packet(CmdType::CLOSE_CONN);
        conn->sendPacket(toClient);
        conn->close();
        pthread_exit(nullptr);
    }

    if (profiles->hasProfile(packet->getPayload())) // if profile exists
    {
        profile = profiles->getProfileById(packet->getPayload());
        if (profile->getSessionsOn() == MAX_SESSIONS)
        {
            cout << "Vai morrer bro" << endl;
            toClient = new Packet(CmdType::CLOSE_CONN);
            conn->sendPacket(toClient);
            conn->close();
            pthread_exit(nullptr);
        }
    }
    else
    {
        profile = new Profile(packet->getPayload());
        profiles->insertProfile(packet->getPayload(), profile);
        cout << "PROFILE " << packet->getPayload() << " has been planted" << endl;
    }

    profile->incSessionsOn(conn);

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
            if (profile->getProfileId() == packet->getPayload())
            {
                cout << "vai dar nao mermao" << endl;
                break;
            }

            cout << profile->getProfileId() << " wants to follow " << packet->getPayload() << endl;

            if (profiles->hasProfile(packet->getPayload()))
            {
                Profile *profileToFollow = profiles->getProfileById(packet->getPayload());
                profileToFollow->addFollower(profile);
            }
            break;
        }
        case CmdType::CLOSE_CONN:
        {
            // concorrencia!!
            // decrease sessionsOn and remove Profile
            if (profile->getSessionsOn() > 0)
            {
                profile->decSessionsOn(conn);
                cout << "Decrementou" << endl;
            }
            clientWantsToQuit = true;
            break;
        }
        case CmdType::SEND:
        {
            for (int i = 0; i < profile->getFollowers()->size(); i++)
            {
                Profile *follower = profile->getFollowers()->at(i);
                cout << "inserindo " << packet->getPayload() << " na inbox de " << follower->getProfileId() << endl;
                follower->sendOrInsertInbox(packet);
            }
            cout << profile->getProfileId() << " says: " << packet->getPayload() << endl;
            break;
        }
        default:
        {
            cout << "I dont know..." << endl;
            break;
        }
        }

        Packet *yep = new Packet(CmdType::SEND, "Yep!");
        conn->sendPacket(yep);
    }

    conn->close();
    pthread_exit(nullptr);
}

int main()
{
    profiles = new ProfilesManager();
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
