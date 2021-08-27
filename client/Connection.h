//
// Created by daniel on 26/08/2021.
//
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

#ifndef TWITTER_CONNECTION_H
#define TWITTER_CONNECTION_H

/* Sockets buffers length */
#define LEN 4096

using namespace std;

class Connection {
public:
    Connection(uint16_t port, const char *server_addr);
    void send_message(const char * msg);
    char * receive_message();
    void close();
    bool is_closed();
private:
    struct sockaddr_in server;
    int sockfd;
    int len = sizeof(server);
    char buffer_in[LEN];
    char buffer_out[LEN];
    bool closed;
};


#endif //TWITTER_CONNECTION_H
