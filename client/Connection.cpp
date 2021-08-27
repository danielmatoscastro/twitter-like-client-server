//
// Created by daniel on 26/08/2021.
//
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

#include "Connection.h"

Connection::Connection(uint16_t port, const char *server_addr) {
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error on client socket creation:");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(server_addr);
    memset(server.sin_zero, 0x0, 8);

    if (connect(sockfd, (struct sockaddr*) &server, len) == -1) {
        perror("Can't connect to server");
        exit(EXIT_FAILURE);
    }

    this->closed = false;
}

bool Connection::is_closed() {
    return this->closed;
}

void Connection::send_message(const char* msg) {
    memset(buffer_out, 0x0, LEN);
    strcpy(buffer_out, msg);

    send(sockfd, buffer_out, strlen(msg)+1, 0);
}

char * Connection::receive_message() {
    memset(buffer_in, 0x0, LEN);
    recv(sockfd, buffer_in, LEN, 0);
    return buffer_in;
}

void Connection::close() {
    if (!this->closed) {
        this->closed = true;
        ::close(sockfd);
    }
}
