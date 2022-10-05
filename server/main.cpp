//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sstream>

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

using namespace std;

Connection *routerConn;
ProfileAccessController *profiles;
Server *server;
vector<ClientConnection *> *listBackups;
vector<string> *electionServerList;
string currentServerAddr;
bool electionStarted = false;
bool backup = false;

void sendToBackups(Packet *packet);

string serverListToString(){
    string result;
    result.clear();

    for (auto str : *electionServerList){
        result.append(str);
        result.append("\n");
    }

    return result;
}

Profile *receiveProfileCmd(ClientConnection *conn)
{
    Profile *profile = nullptr;
    Packet *toClient;
    Packet *packet = conn->receivePacket();

    if (packet->getCmd() != CmdType::PROFILE && packet->getCmd() != CmdType::SET_BACKUP)
    {
        profile = profiles->getProfileById(packet->getSender());
        profile->updateSessionsOn(conn);
    }

    if (packet->getCmd() == CmdType::SET_BACKUP)
    {
        listBackups->push_back(conn);
        electionServerList->push_back(packet->getPayload());
        sendToBackups(new Packet(CmdType::BACKUP_PROPAGATION, serverListToString()));
    }
    else if (packet->getCmd() == CmdType::PROFILE)
    {
        profile = profiles->createProfileIfNotExists(packet->getPayload(), conn);
        sendToBackups(packet);
    }

    return profile;
}

void sendToBackups(Packet *packet)
{
    for (int i = 0; i < listBackups->size(); i++)
    {
        listBackups->at(i)->sendPacket(packet);
    }
}

void sendElectionMsg(string serverAddr){
    electionStarted = true;
    auto itr = std::find(electionServerList->begin(), electionServerList->end(), currentServerAddr);

    int index=0;
    if (itr != electionServerList->cend()) {
        index = std::distance(electionServerList->begin(), itr);
        std::cout << "Element present at index " << index;

        //send to next server in the ring
        int nextServer = (index+1)%electionServerList->size();

        string addrAndPort = electionServerList->at(nextServer);

        size_t pos = addrAndPort.find(':');
        string addr = addrAndPort.substr(0, pos);
        string port = addrAndPort.substr(pos + 1);

        Connection *nextElectionServer = new Connection(stoi(port), addr.c_str());
        Packet *electionPacket = new Packet(CmdType::ELECTION, serverAddr);
        nextElectionServer->sendPacket(electionPacket);
        
        nextElectionServer->sendPacket(new Packet(CmdType::CLOSE_CONN, ""));
        nextElectionServer->close();
    }
    else{
        std::cout << "Element not found" << endl;
    }
}

void sendElectedMsg(string serverAddr, bool setPrimary){
    electionStarted = false;

    auto itr = std::find(electionServerList->begin(), electionServerList->end(), currentServerAddr);
    int index=0;
    if (itr != electionServerList->cend()) {
        index = std::distance(electionServerList->begin(), itr); //index = itr - electionServerList->cbegin(); 
        std::cout << "(In if(is Primary)) Element present at index " << index;
    
        if(setPrimary){
            //remove the current primary server from the list
            // REVER index após essa exclusão
            electionServerList->erase(itr);
            
            cout << "Standard exception RECEIVE_ALIVE: " << endl;
            routerConn = new Connection(3000, "127.0.0.1");
            routerConn->sendPacket(new Packet(CmdType::SET_PRIMARY, currentServerAddr));
            Packet *routerResponse = routerConn->receivePacket();
            cout << "routerResponse: " << routerResponse->getCmd() << endl;
            routerConn->sendPacket(new Packet(CmdType::CLOSE_CONN));
            routerConn->close();
        }

        //send elected message to next server in the ring
        int nextServer = (index+1)%electionServerList->size();

        string addrAndPort = electionServerList->at(nextServer);

        size_t pos = addrAndPort.find(':');
        string addr = addrAndPort.substr(0, pos);
        string port = addrAndPort.substr(pos + 1);

        Connection *nextElectionServer = new Connection(stoi(port), addr.c_str());
        Packet *electedPacket = new Packet(CmdType::ELECTED, serverAddr);
        nextElectionServer->sendPacket(electedPacket);

        nextElectionServer->sendPacket(new Packet(CmdType::CLOSE_CONN, ""));
        nextElectionServer->close();
    }              
    else {
        std::cout << "(In if(is Primary)) Element not found";
    }

}

bool processPacket(ClientConnection *conn, Profile *profile, Packet *packet)
{
    bool clientWantsToQuit = false;

    switch (packet->getCmd())
    {
    case CmdType::PROFILE:
    {
        profiles->createProfileIfNotExists(packet->getPayload(), conn);
        sendToBackups(packet);
        break;
    }
    case CmdType::FOLLOW:
    {
        profiles->addFollowerTo(packet->getPayload(), profile);
        sendToBackups(packet);
        break;
    }
    case CmdType::CLOSE_CONN:
    {
        profile->decSessionsOn(conn);
        clientWantsToQuit = true;
        sendToBackups(packet);
        break;
    }
    case CmdType::SEND:
    {
        profiles->sendToFollowersOf(profile, packet);
        sendToBackups(packet);
        break;
    }
    case CmdType::ALIVE:
    {
        break;
    }
    case CmdType::BACKUP_PROPAGATION:
    {   
        std::string strData = packet->getPayload();
        std::stringstream streamData(strData);     
        std::string val;
        electionServerList->clear();
        while (std::getline(streamData, val)) {
            electionServerList->push_back(val);
        }
        break;
    }
    case CmdType::ELECTION:
    {
        // int idComp = strcmp(packet->getPayload().c_str(), currentServerAddr.c_str());

        // //if msg id > id -> send; electionStarted
        // //else msg id < id && !electionStarted; -> send(id); electionStarted
        // //else msg id < id && electionStarted -> do nothing
        // //else msg id = id -> isPrimary = true;

        // if(idComp > 0){
        //     sendElectionMsg(packet->getPayload());
        // }
        // else if(idComp < 0 && !electionStarted){
        //     sendElectionMsg(currentServerAddr);
        // }
        // else if(idComp == 0){ // if msg id == current server id => current server was elected as primary
        //     sendElectedMsg(currentServerAddr, true);
        // }

        break;
    }
    case CmdType::ELECTED:
    {
        // int idComp = strcmp(packet->getPayload().c_str(), currentServerAddr.c_str());


        // if(idComp != 0){ // if msg id != id => current server isn't the elected server, so propagate message forward 
        //     sendElectedMsg(currentServerAddr, false);
        //     backup = true;
        // }
        // else{
        //     backup = false;
        // }

        break;
    }
    default:
    {
        break;
    }
    }

    return clientWantsToQuit;
}

void *fromClient(void *_conn)
{
    ClientConnection *conn = (ClientConnection *)_conn;
    Profile *profile = receiveProfileCmd(conn);

    if (profile == nullptr)
    {
        return nullptr;
    }

    Packet *hello = new Packet(CmdType::SEND, "Hello client! " + profile->getProfileId());
    conn->sendPacket(hello);
    profile->fetchInboxContent();

    cout << "Client " << profile->getProfileId() << " connected." << endl
         << "Waiting for client message ..." << endl;

    bool clientWantsToQuit = false;
    while (!clientWantsToQuit && !conn->isClosed())
    {
        Packet *packet = conn->receivePacket();

        clientWantsToQuit = processPacket(conn, profile, packet);
    }

    conn->close();
    pthread_exit(nullptr);
}

void *sendAlive(void *_conn)
{
    while (true)
    {
        for (int i = 0; i < listBackups->size(); i++)
        {
            cout << i << endl;
        }
    }
}

void *receiveAlive(void *_conn)
{
    Connection *primary_conn = (Connection *)_conn;

    while (true)
    {
        try
        {
            Packet *packet = primary_conn->receivePacket();
            Profile *profile = profiles->getProfileById(packet->getSender());
            processPacket(nullptr, profile, packet);
        }
        catch (...)
        {
            // Entra aqui quando o server for desligado (simulação de um crash)
            routerConn = new Connection(3000, "127.0.0.1");
            routerConn->sendPacket(new Packet(CmdType::SET_PRIMARY, currentServerAddr));
            Packet *routerResponse = routerConn->receivePacket();
            routerConn->sendPacket(new Packet(CmdType::CLOSE_CONN));
            routerConn->close();

            // Se está em processo de/começou a eleição => ignora outras eleiçẽos iniciadas a seguir
            // if(!electionStarted){
            //     sendElectionMsg(currentServerAddr);
            //}
            pthread_exit(NULL);
        }
    }
}

void interruptionHandler(sig_atomic_t sigAtomic)
{
    server->close();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "too few arguments" << endl;
        exit(EXIT_FAILURE);
    }

    char *addr = argv[1];
    char *port = argv[2];

    profiles = new ProfileAccessController("state.json");
    listBackups = new vector<ClientConnection *>();
    electionServerList = new vector<string>();
    signal(SIGINT, interruptionHandler);

    currentServerAddr.clear();
    currentServerAddr.append(addr);
    currentServerAddr.append(":");
    currentServerAddr.append(port);

    pthread_t *receiveAliveTh;
    pthread_t *sendAliveTh;

    routerConn = new Connection(3000, "127.0.0.1");
    routerConn->sendPacket(new Packet(CmdType::SET_PRIMARY_IF_NOT_EXISTS, currentServerAddr));
    Packet *routerResponse = routerConn->receivePacket();

    string payload = routerResponse->getPayload();
        
    switch (routerResponse->getCmd())
    {
        case CmdType::SET_PRIMARY:
            // if Server addr returned == current server addr => current server is primary
            if(strcmp(payload.c_str(), currentServerAddr.c_str()) == 0){
                cout << "Sou primario" << endl;
                backup = false;
            }
            else{
                cout << "Sou backup!" << endl;
                backup = true;
            }
            break;
        case CmdType::OK:
            cout << "Sou primario!" << endl;
            backup = false;
            break;
        }

        routerConn->sendPacket(new Packet(CmdType::CLOSE_CONN));
        routerConn->close();

        if (backup)
        {
            size_t pos = payload.find(':');
            string addr = payload.substr(0, pos);
            string port = payload.substr(pos + 1);

            Connection *primaryConn = new Connection(stoi(port), addr.c_str());
            primaryConn->sendPacket(new Packet(CmdType::SET_BACKUP, currentServerAddr));

            receiveAliveTh = new pthread_t();
            if (pthread_create(receiveAliveTh, NULL, receiveAlive, primaryConn) != 0)
            {
                perror("pthread_create error:");
                return EXIT_FAILURE;
            }

            pthread_join(*receiveAliveTh, NULL);

        }

    server = new Server(stoi(port));
    sendAliveTh = new pthread_t();
    if (pthread_create(sendAliveTh, NULL, sendAlive, NULL) != 0)
    {
        perror("pthread_create error:");
        return EXIT_FAILURE;
    }

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
