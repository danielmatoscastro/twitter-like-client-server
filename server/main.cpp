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
#include <map>
#include "Profile.h"

#define PORT 4242
#define MAX_SESSIONS 2

using namespace std;

map<string, Profile *> *profiles;

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

    bool isInMap = profiles->count(packet->getPayload()) > 0;
    if (isInMap)
    {
        profile = profiles->find(packet->getPayload())->second;
        if (profile->getSessionsOn() == MAX_SESSIONS)
        {
            cout << "Vai morrer bro" << endl;
            toClient = new Packet(CmdType::CLOSE_CONN);
            conn->sendPacket(toClient);
            conn->close();
            pthread_exit(nullptr);
        }

        profile->incSessionsOn();
    }
    else
    {
        profile = new Profile(packet->getPayload());
        profiles->insert(make_pair(packet->getPayload(), profile));
        cout << "PROFILE " << packet->getPayload() << " has been planted" << endl;
    }

    return profile;
}

void *from_client(void *_conn)
{
    ClientConnection *conn = (ClientConnection *)_conn;

    Profile *profile = receiveProfileCmd(conn);

    Packet *hello = new Packet("Hello client! " + profile->getProfileId());
    conn->sendPacket(hello);

    cout << "Client " << profile->getProfileId() << " connected." << endl
         << "Waiting for client message ..." << endl;

    do
    {
        Packet *packet = conn->receivePacket();

        if (packet->getCmd() == CmdType::FOLLOW)
        {
            cout << profile->getProfileId() << " wants to follow " << packet->getPayload() << endl;
            
            bool isInMap = profiles->count(packet->getPayload()) > 0;
            if(isInMap)
            {
                Profile *profileToFollow = profiles->find(packet->getPayload())->second;
                profileToFollow->addFollower(profile);
            }
        }
        else if (packet->getCmd() == CmdType::CLOSE_CONN)
        {
            // concorrencia!! 
            // decrease sessionsOn and remove Profile
            if(profile->getSessionsOn() > 0){
                profile->decSessionsOn();
                cout << "Decrementou" << endl;
            }
            break;
        }
        else
        {
            cout << profile->getProfileId() << " says: " << packet->getPayload() << endl;
        }

        Packet *yep = new Packet("Yep!");
        conn->sendPacket(yep);
    } while (true);

    conn->close();
    pthread_exit(nullptr);
}

int main()
{
    profiles = new map<string, Profile *>();
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
