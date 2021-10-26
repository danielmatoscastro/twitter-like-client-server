#include <iostream>
#include <iomanip>
#include <string>
#include <signal.h>
#include <pthread.h>
#include "../commons/Connection.h"
#include "../commons/Packet.h"

#define MAX_MSG_LEN 128

Connection *con;
pthread_t to_server_th;
pthread_t from_server_th;
pthread_t control_thread_th;

void interruption_handler(sig_atomic_t sigAtomic)
{
    Packet *packet = new Packet(CmdType::CLOSE_CONN);
    con->sendPacket(packet);
    con->close();
    exit(EXIT_SUCCESS);
}

Connection *routerConn;
string primary = "random:primary";

void send_presentation(char *profile)
{
    Packet *packet = new Packet(CmdType::PROFILE, profile);
    con->sendPacket(packet);
}

void updateConn()
{
    cout << "hey" << endl;

    Packet *IP_request = new Packet(CmdType::GET_PRIMARY);
    routerConn->sendPacket(IP_request);
    Packet *Response = routerConn->receivePacket();

    string payload = Response->getPayload();

    if (payload.compare(primary) != 0)
    {
        // formato "addr:port"
        size_t pos = payload.find(':');
        string addr = payload.substr(0, pos);
        string port = payload.substr(pos + 1);

        if (con)
        {
            con->close();
        }

        con = new Connection(stoi(port), addr.c_str());
    }
}

void *to_server(void *args)
{
    string line;
    char *profile = (char *)args;

    updateConn();

    send_presentation(profile);

    cout << "aqui" << endl;
    getline(cin, line);
    while (!cin.eof())
    {

        cout << "oi" << endl;
        if (con == nullptr || !con->isClosed())
        {
            updateConn();

            Packet *packet;
            if (line.rfind("FOLLOW") == 0)
            {
                packet = new Packet(CmdType::FOLLOW, line.substr(7));
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

            con->sendPacket(packet);
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

void *from_server(void *args)
{
    updateConn();
    while (!con->isClosed())
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
            ss << put_time(localtime(&time), "%b %d %H:%M:%S %Y");
            cout << packet->getSender() << ": " << packet->getPayload() << " at: " << ss.str() << endl;
        }
    }

    pthread_exit(NULL);
}

void *control_thread(void *args)
{
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "too few arguments" << endl;
        exit(EXIT_FAILURE);
    }

    char *profile = argv[1];
    char *addr = argv[2];
    char *port = argv[3];

    if (strlen(profile) < 5 || strlen(profile) > 21)
    {
        cout << "profile must have [4-20] chars." << endl;
        exit(EXIT_FAILURE);
    }

    cout << "ta de brinks" << endl;

    routerConn = new Connection(stoi(port), addr);
    // con = new Connection(stoi(port), addr);

    cout << "claro que to" << endl;

    signal(SIGINT, interruption_handler);

    pthread_create(&to_server_th, NULL, to_server, profile);
    pthread_create(&from_server_th, NULL, from_server, NULL);

    pthread_join(to_server_th, NULL);
    pthread_join(from_server_th, NULL);

    return EXIT_SUCCESS;
}
