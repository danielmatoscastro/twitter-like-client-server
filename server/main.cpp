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
vector<ClientConnection *> *listConnections;

Profile *receiveProfileCmd(ClientConnection *conn)
{
    Profile *profile;
    Packet *toClient;
    Packet *packet = conn->receivePacket();
    if (packet->getCmd() != CmdType::PROFILE && packet->getCmd() != CmdType::SET_BACKUP)
    {
        toClient = new Packet(CmdType::CLOSE_CONN);
        conn->sendPacket(toClient);
        conn->close();
        pthread_exit(nullptr);
    }
    if (packet->getCmd() == CmdType::SET_BACKUP)
    {
        cout << "Entrei no set backup" << endl;
        // formato "addr:port"
        //string payload = packet->getPayload();
        //size_t pos = payload.find(':');
        //string addr = payload.substr(0, pos);
        //string port = payload.substr(pos + 1);

        listConnections->push_back(conn);
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
        case CmdType::ALIVE:
        {
            cout << "Vivo" << endl;
            break;
        }
        default:
        {
            cout << "Print getcmd: " << packet->getCmd() << endl;
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

void *send_alive(void *_conn)
{
    while (true)
    {
        //cout << "Tamanho lista: " << listConnections->size() << endl;
        for (int i = 0; i < listConnections->size(); i++)
        {
            cout << i << endl;
            //listConnections->at(i)->sendPacket(new Packet(CmdType::ALIVE));
        }
    }
}

void *receive_alive(void *_conn)
{
    Connection *primary_conn = (Connection *)_conn;

    while (true)
    {
        try
        {
            Packet *packet = primary_conn->receivePacket();
            cout << "Dentro do receive alive" << endl;
        }
        catch (...)
        {
            //Entra aqui quando o server for desligado (simulação de um crash)
            cout << "Standard exception RECEIVE_ALIVE: " << endl;
            pthread_exit(NULL);
        }
    }
}

void interruptionHandler(sig_atomic_t sigAtomic)
{
    //profiles->sendCloseConnToAll();
    server->close();
    exit(EXIT_SUCCESS);
}

int main()
{
    listConnections = new vector<ClientConnection *>();
    signal(SIGINT, interruptionHandler);
    routerConn = new Connection(3000, "127.0.0.1");
    routerConn->sendPacket(new Packet(CmdType::SET_PRIMARY_IF_NOT_EXISTS, "127.0.0.1:4242"));
    Packet *routerResponse = routerConn->receivePacket();

    bool backup = false;

    switch (routerResponse->getCmd())
    {
    case CmdType::SET_PRIMARY:
        cout << "Sou backup!" << endl;
        backup = true;
        break;
    case CmdType::OK:
        cout << "Sou primario!" << endl;
        break;
    }

    routerConn->sendPacket(new Packet(CmdType::CLOSE_CONN));
    routerConn->close();

    pthread_t *receiveAlive_th;
    pthread_t *sendAlive_th;

    if (backup)
    {
        string payload = routerResponse->getPayload();
        cout << "Payload: " << payload << endl;
        size_t pos = payload.find(':');
        string addr = payload.substr(0, pos);
        string port = payload.substr(pos + 1);

        cout << "Port: " << port << endl;
        cout << "Addr: " << addr << endl;
        Connection *primary_conn = new Connection(stoi(port), addr.c_str());
        primary_conn->sendPacket(new Packet(CmdType::SET_BACKUP, "127.0.0.1:5000"));

        sleep(1);

        receiveAlive_th = new pthread_t();
        if (pthread_create(receiveAlive_th, NULL, receive_alive, primary_conn) != 0)
        {
            perror("pthread_create error:");
            return EXIT_FAILURE;
        }

        //sleep(10000);
        pthread_join(*receiveAlive_th, NULL);
    }

    cout << "Passei do if" << endl;
    profiles = new ProfileAccessController("state.json");
    server = new Server(PORT);
    cout << "Passei do server" << endl;
    sendAlive_th = new pthread_t();
    if (pthread_create(sendAlive_th, NULL, send_alive, NULL) != 0)
    {
        perror("pthread_create error:");
        return EXIT_FAILURE;
    }

    while (true)
    {
        cout << "Entrei no while" << endl;
        ClientConnection *conn = server->waitClient();
        cout << "Recebi um cliente" << endl;
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
