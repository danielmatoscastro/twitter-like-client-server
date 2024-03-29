#include <iostream>
#include <iomanip>
#include <string>
#include <signal.h>
#include <pthread.h>
#include "../commons/Connection.h"
#include "../commons/Packet.h"

#define MAX_MSG_LEN 128

Connection *con;
Connection *routerConn;
char *profile;
string primary = "random:primary";

pthread_t toServerTh;
pthread_t fromServerTh;

void interruptionHandler(sig_atomic_t sigAtomic)
{
    cout << "sigkill" << endl;

    std::string prof = std::string(profile);
    Packet *packet = new Packet(CmdType::CLOSE_CONN, "", prof);
    con->sendPacket(packet);
    con->close();
    routerConn->sendPacket(packet);
    routerConn->close();

    //kill to avoid useless threads running
    pthread_kill(toServerTh, SIGKILL);
    pthread_kill(fromServerTh, SIGKILL);
}

void sendPresentation(char *profile)
{
    Packet *packet = new Packet(CmdType::PROFILE, profile);
    con->sendPacket(packet);
}

pthread_mutex_t updateMutex = PTHREAD_MUTEX_INITIALIZER;

void updateConn(bool sendOk)
{
    pthread_mutex_lock(&updateMutex);

    Packet *IP_request = new Packet(CmdType::GET_PRIMARY);
    routerConn->sendPacket(IP_request);
    Packet *Response = routerConn->receivePacket();

    string payload = Response->getPayload();
    if (payload.compare(primary) != 0)
    {
        primary = payload;
        // formato "addr:port"
        size_t pos = payload.find(':');
        string addr = payload.substr(0, pos);
        string port = payload.substr(pos + 1);

        if (con)
        {
            con->close();
        }

        bool retry = true;
        while (retry)
        {
            try
            {
                con = new Connection(stoi(port), addr.c_str());
                retry = false;

                if(sendOk){
                    // Sending profile to new Primary server (to update the session/conection)
                    con->sendPacket(new Packet(CmdType::OK, "", profile));
                }
            }
            catch (...)
            {
            }
        }
    }
    pthread_mutex_unlock(&updateMutex);
}

void *toServer(void *args)
{
    string line;
    char *profile = (char *)args;

    sendPresentation(profile);

    getline(cin, line);
    while (!cin.eof())
    {
        if(con == nullptr || !con->isClosed())
        {
            Packet *packet;
            if (line.rfind("FOLLOW") == 0)
            {
                packet = new Packet(CmdType::FOLLOW, line.substr(7), profile);
            }
            else if (line.rfind("SEND") == 0)
            {
                string message = line.substr(5);
                if (message.size() > MAX_MSG_LEN)
                {
                    cout << "Too long... max 128 chars." << endl;
                    continue;
                }
                packet = new Packet(CmdType::SEND, line.substr(5), profile);
            }
            else
            {
                cout << "Sorry, wrong command..." << endl;
                continue;
            }

            try
            {
                con->sendPacket(packet);
            }
            catch (...)
            {
                updateConn(true);
            
            }
        }
        else
        {
            pthread_exit(NULL);
        }

        getline(cin, line);
    }

    con->sendPacket(new Packet(CmdType::CLOSE_CONN));
    con->close();
    exit(EXIT_SUCCESS);
}

void *fromServer(void *args)
{
    while (!con->isClosed())
    {
        try
        {
            Packet *packet = con->receivePacket();
            if (packet->getCmd() == CmdType::CLOSE_CONN)
            {
                con->close();
                cout << "Server sent CLOSE_CONN" << endl;
                exit(EXIT_SUCCESS);
            }
            else
            {
                stringstream ss;
                time_t time = packet->getTimestamp();
                ss << put_time(localtime(&time), "%d/%m/%Y %H:%M");
                cout << "(" << ss.str() << ") " << packet->getSender() << ": " << packet->getPayload() << endl;
            }
        }
        catch (...)
        {
            // Entra aqui quando o server for desligado (simulação de um crash)
            updateConn(true);

        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "too few arguments" << endl;
        exit(EXIT_FAILURE);
    }

    profile = argv[1];
    char *addr = argv[2];
    char *port = argv[3];

    if (strlen(profile) < 5 || strlen(profile) > 21)
    {
        cout << "profile must have [4-20] chars." << endl;
        exit(EXIT_FAILURE);
    }

    routerConn = new Connection(stoi(port), addr);

    signal(SIGINT, interruptionHandler);

    updateConn(false);
    pthread_create(&toServerTh, NULL, toServer, profile);
    pthread_create(&fromServerTh, NULL, fromServer, NULL);

    pthread_join(toServerTh, NULL);
    pthread_join(fromServerTh, NULL);

    return EXIT_SUCCESS;
}
