#include <iostream>
#include <string>
#include <signal.h>
#include <pthread.h>
#include "Connection.h"

Connection *con;
pthread_t to_server_th;
pthread_t from_server_th;

void interruption_handler(sig_atomic_t sigAtomic)
{
    con->close();
    exit(EXIT_SUCCESS);
}

void *to_server(void *args)
{
    std::string line;
    std::getline(std::cin, line);
    while (!cin.eof())
    {
        if (!con->is_closed())
        {
            con->send_message(line.c_str());
            std::getline(std::cin, line);
        }
        else
        {
            pthread_exit(NULL);
        }
    }

    con->close();
    pthread_exit(NULL);
}

void *from_server(void *args)
{
    while (!con->is_closed())
    {
        cout << con->receive_message();
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        perror("too few arguments\n");
        exit(EXIT_FAILURE);
    }

    char *profile = argv[1];
    char *addr = argv[2];
    char *port = argv[3];
    con = new Connection(stoi(port), addr);

    signal(SIGINT, interruption_handler);

    std::string presentation = "PROFILE " + string(profile);
    con->send_message(presentation.c_str());

    pthread_create(&to_server_th, NULL, to_server, NULL);
    pthread_create(&from_server_th, NULL, from_server, NULL);

    pthread_join(to_server_th, NULL);

    return EXIT_SUCCESS;
}
