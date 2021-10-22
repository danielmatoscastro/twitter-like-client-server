//
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
#include <signal.h>
#include <iostream>
#include "../commons/Packet.h"

#define PORT 4242

using namespace std;

void interruption_handler(sig_atomic_t sigAtomic)
{
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, interruption_handler);

    cout << "DNS is running" << endl;

    return EXIT_SUCCESS;
}
