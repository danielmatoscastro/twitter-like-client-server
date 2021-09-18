#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <iostream>
#include "Server.h"

Server::Server(uint16_t port)
{
    cout << "Starting server... ";

    this->serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->serverfd == -1)
    {
        perror("Can't create the server socket:");
        exit(EXIT_FAILURE);
    }
    cout << "Server on." << endl;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(server.sin_zero, 0, 8);

    int yes = 1;
    if (setsockopt(this->serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Socket options error:");
        exit(EXIT_FAILURE);
    }

    if (bind(this->serverfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("Socket bind error:");
        exit(EXIT_FAILURE);
    }

    if (listen(this->serverfd, BACKLOG_SIZE) == -1)
    {
        perror("Listen error:");
        exit(EXIT_FAILURE);
    }

    cout << "Listening on port " << port << endl;

    this->serverfd = serverfd;
}

void Server::close()
{
    ::close(this->serverfd);
}

ClientConnection *Server::waitClient()
{
    struct sockaddr_in *client = new struct sockaddr_in();
    socklen_t client_len = sizeof(client);
    int clientfd = accept(serverfd, (struct sockaddr *)client, &client_len);

    if (clientfd == -1)
    {
        perror("Accept error:");
        exit(EXIT_FAILURE);
    }

    return new ClientConnection(clientfd);
}
