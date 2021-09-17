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

/* Server port  */
#define PORT 4242

using namespace std;

void *from_client(void *_conn)
{
    ClientConnection *conn = (ClientConnection *)_conn;

    Packet *hello = new Packet("Hello client!");
    conn->sendPacket(hello);

    cout << "Client connected." << endl
         << "Waiting for client message ..." << endl;

    do
    {
        Packet *packet = conn->receivePacket();
        cout << "Client says: " << packet->getPayload() << endl;

        Packet *yep = new Packet("Yep!");
        conn->sendPacket(yep);
    } while (true);

    conn->close();
}

int main()
{
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
