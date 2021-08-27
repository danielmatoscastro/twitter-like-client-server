#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <string>
#include <sstream>

#include "Connection.h"

/* Defines the server port */
#define PORT 4242

/* Server address */
#define SERVER_ADDR "127.0.0.1"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        perror("too few arguments\n");
        exit(EXIT_FAILURE);
    }

    char * perfil = argv[1];
    char * addr = argv[2];
    char * port = argv[3];

    Connection * con = new Connection(PORT, SERVER_ADDR);
    std::stringstream ss;
    ss << "PERFIL " << perfil;
    con->send_message(ss.str().c_str());

    std::string line;
    while (true) {
        string message;
        cin >> message;

        std::getline(std::cin, line);
        if (message == "exit") {
            break;
        }

        printf(con->send_message(line.c_str()));
    }

    con->close();

    fprintf(stdout, "\nConnection closed\n\n");

    return EXIT_SUCCESS;
}
