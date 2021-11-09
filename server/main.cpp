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

//#define PORT 4242

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
    cout << "packet primeirao " << packet->getPayload() << endl;
    if (packet->getCmd() != CmdType::PROFILE && packet->getCmd() != CmdType::SET_BACKUP)
    {
        profile = profiles->getProfileById(packet->getSender());
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
        cout << "enviando para backup" << endl;
        listBackups->at(i)->sendPacket(packet);
    }
}

void sendElectionMsg(string serverAddr){
    electionStarted = true;
    auto itr = std::find(electionServerList->begin(), electionServerList->end(), currentServerAddr);

    int index=0;
    if (itr != electionServerList->cend()) {
        index = itr - electionServerList->cbegin();// index = std::distance(electionServerList->begin(), itr);
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

bool processPacket(ClientConnection *conn, Profile *profile, Packet *packet)
{
    bool clientWantsToQuit = false;

    // cout << "processPacket" << endl;
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
        cout << "FOLLOW" << endl;
        profiles->addFollowerTo(packet->getPayload(), profile);
        sendToBackups(packet);
        break;
    }
    case CmdType::CLOSE_CONN:
    {
        cout << "CLOSE_CONN" << endl;
        profile->decSessionsOn(conn);
        clientWantsToQuit = true;
        sendToBackups(packet);
        break;
    }
    case CmdType::SEND:
    {
        cout << "SEND" << endl;
        profiles->sendToFollowersOf(profile, packet);
        sendToBackups(packet);
        break;
    }
    case CmdType::ALIVE:
    {
        cout << "Vivo" << endl;
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
        cout << "ElectionList propagated:\n" << packet->getPayload() << endl;
        break;
    }
    case CmdType::ELECTION:
    {
        int idComp = strcmp(packet->getPayload().c_str(), currentServerAddr.c_str());

        //if msg id > id -> send; electionStarted
        //else msg id < id && !electionStarted; -> send(id); electionStarted
        //else msg id < id && electionStarted -> do nothing
        //else msg id = id -> isPrimary = true;

        if(idComp > 0){
            sendElectionMsg(packet->getPayload());
        }
        else if(idComp < 0 && !electionStarted){
            sendElectionMsg(currentServerAddr);
        }
        else if(idComp == 0){ // if msg id == current server id => server was elected as primary

            electionStarted = false;

            auto itr = std::find(electionServerList->begin(), electionServerList->end(), currentServerAddr);
            int index=0;
            if (itr != electionServerList->cend()) {
                index = itr - electionServerList->cbegin(); //index = std::distance(electionServerList->begin(), itr);
                std::cout << "(In if(is Primary)) Element present at index " << index;
            

                //remove the current primary server from the list
                electionServerList->erase(itr);
                
                cout << "Standard exception RECEIVE_ALIVE: " << endl;
                routerConn = new Connection(3000, "127.0.0.1");
                routerConn->sendPacket(new Packet(CmdType::SET_PRIMARY, currentServerAddr));
                Packet *routerResponse = routerConn->receivePacket();
                cout << "routerResponse: " << routerResponse->getCmd() << endl;
                routerConn->sendPacket(new Packet(CmdType::CLOSE_CONN));
                routerConn->close();


                //send elected message to next server in the ring
                int nextServer = (index+1)%electionServerList->size();

                string addrAndPort = electionServerList->at(nextServer);

                size_t pos = addrAndPort.find(':');
                string addr = addrAndPort.substr(0, pos);
                string port = addrAndPort.substr(pos + 1);

                Connection *nextElectionServer = new Connection(stoi(port), addr.c_str());
                Packet *electedPacket = new Packet(CmdType::ELECTED, currentServerAddr);
                nextElectionServer->sendPacket(electedPacket);

                nextElectionServer->sendPacket(new Packet(CmdType::CLOSE_CONN, ""));
                nextElectionServer->close();
            }              
            else {
                std::cout << "(In if(is Primary)) Element not found";
            }
        }

        break;
    }
    case CmdType::ELECTED:
    {
        int idComp = strcmp(packet->getPayload().c_str(), currentServerAddr.c_str());


        if(idComp != 0){
            electionStarted = false;

            auto itr = std::find(electionServerList->begin(), electionServerList->end(), currentServerAddr);
            int index=0;
            if (itr != electionServerList->cend()) {
                index = itr - electionServerList->cbegin(); //index = std::distance(electionServerList->begin(), itr);
                std::cout << "(In if(is Primary)) Element present at index " << index;


                //send elected message to next server in the ring
                int nextServer = (index+1)%electionServerList->size();

                string addrAndPort = electionServerList->at(nextServer);

                size_t pos = addrAndPort.find(':');
                string addr = addrAndPort.substr(0, pos);
                string port = addrAndPort.substr(pos + 1);

                Connection *nextElectionServer = new Connection(stoi(port), addr.c_str());
                Packet *electedPacket = new Packet(CmdType::ELECTED, currentServerAddr);
                nextElectionServer->sendPacket(electedPacket);

                nextElectionServer->sendPacket(new Packet(CmdType::CLOSE_CONN, ""));
                nextElectionServer->close();

                backup = true;

            }              
            else {
                std::cout << "(In if(is Primary)) Element not found";
            }
        }
        else{
            backup = false;
        }

        break;
    }
    default:
    {
        // cout << "Print getcmd: " << packet->getCmd() << endl;
        // cout << "I dont know..." << endl;
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
        // cout << "entrou aqui" << endl;
        Packet *packet = conn->receivePacket();

        // cout << "Payload: " << packet->getPayload() << endl;

        clientWantsToQuit = processPacket(conn, profile, packet);

        // Packet *yep = new Packet(CmdType::SEND, "Yep!", "server");
        // conn->sendPacket(yep);
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
            // listConnections->at(i)->sendPacket(new Packet(CmdType::ALIVE));
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
            cout << packet->getCmd() << " " << packet->getPayload() << endl;
            Profile *profile = profiles->getProfileById(packet->getSender());
            cout << "chamando processPacket " << profile << " sender:" << packet->getSender() << endl;
            processPacket(nullptr, profile, packet);
        }
        catch (...)
        {
            // Entra aqui quando o server for desligado (simulação de um crash)
            cout << "Entrou no catch" << endl;
            // Se está em processo de/começou a eleição => ignora outras eleiçẽos iniciadas a seguir
            if(!electionStarted){
                electionStarted = true;

                cout << "Entrei no if" << endl;
                //find current server in electionServerList
                auto itr = std::find(electionServerList->begin(), electionServerList->end(), currentServerAddr);
                cout << "Passou iterator" << endl;
                int index = 0;
                if (itr != electionServerList->end()) {
                    cout << "entrou no if 3" << endl;
                    //index = itr - electionServerList->cbegin(); //std::distance(electionServerList->cbegin(), itr);
                    //cout << "Element present at index " << index;

                    //send to next server in the ring
                    int nextServer = (index+1)%(electionServerList->size());

                    string addrAndPort = electionServerList->at(nextServer);

                    size_t pos = addrAndPort.find(':');
                    string addr = addrAndPort.substr(0, pos);
                    string port = addrAndPort.substr(pos + 1);

                    Connection *nextElectionServer = new Connection(stoi(port), addr.c_str());
                    Packet *electionPacket = new Packet(CmdType::ELECTION, currentServerAddr);
                    nextElectionServer->sendPacket(electionPacket);
                    
                    nextElectionServer->sendPacket(new Packet(CmdType::CLOSE_CONN, ""));
                    nextElectionServer->close();
                }
                else {
                   cout << "Element not found";
                }
            }
            cout << "vai fechar a thread" << endl;
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
    routerConn = new Connection(3000, "127.0.0.1");

    currentServerAddr.clear();
    currentServerAddr.append(addr);
    currentServerAddr.append(":");
    currentServerAddr.append(port);

    pthread_t *receiveAliveTh;
    pthread_t *sendAliveTh;

    do{

        routerConn->sendPacket(new Packet(CmdType::SET_PRIMARY_IF_NOT_EXISTS, currentServerAddr));
        Packet *routerResponse = routerConn->receivePacket();

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

        if (backup)
        {
            string payload = routerResponse->getPayload();
            cout << "Payload: " << payload << endl;
            size_t pos = payload.find(':');
            string addr = payload.substr(0, pos);
            string port = payload.substr(pos + 1);

            cout << "Port: " << port << endl;
            cout << "Addr: " << addr << endl;
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
    }while(backup);

    cout << "Agora sou primario" << endl;

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
