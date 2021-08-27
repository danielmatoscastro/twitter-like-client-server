#include <iostream>
#include <string>
#include <signal.h>
#include <pthread.h>
#include "Connection.h"

Connection * con;

void interruption_handler(sig_atomic_t sigAtomic){
    con->close();
    fprintf(stdout, "\nConnection closed\n\n");
    exit(EXIT_SUCCESS);
}

void * to_server(void * args) {
    std::string line;
    std::getline(std::cin, line);
    while (!con->is_closed() && !cin.eof()) {
        con->send_message(line.c_str());
        std::getline(std::cin, line);
    }
    con->close();
    cout << "retornei" << endl;
    return NULL;
}

void * from_server(void * args) {
    while (!con->is_closed()) {
        cout << "entrei" << endl;
        cout << con->receive_message();
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        perror("too few arguments\n");
        exit(EXIT_FAILURE);
    }

    char * profile = argv[1];
    char * addr = argv[2];
    char * port = argv[3];
    con = new Connection(stoi(port), addr);

    signal(SIGINT, interruption_handler);

    std::string presentation = "PROFILE " + string(profile);
    con->send_message(presentation.c_str());

    pthread_t to_server_th;
    pthread_t from_server_th;

    pthread_create(&to_server_th, NULL, to_server, NULL);
    pthread_create(&from_server_th, NULL, from_server, NULL);

    pthread_join(to_server_th, NULL);
    pthread_join(from_server_th, NULL);

    fprintf(stdout, "\nConnection closed\n\n");
    return EXIT_SUCCESS;
}
