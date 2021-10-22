#ifndef TWITTER_PROFILE_H
#define TWITTER_PROFILE_H

#include <string>
#include <vector>
#include <pthread.h>
#include "../commons/Packet.h"
#include "../commons/ClientConnection.h"
#include "Inbox.h"

using namespace std;

class Profile
{
public:
    Profile(string profile_id);
    string getProfileId();
    int getSessionsOn();
    vector<Profile *> *getFollowers();
    void incSessionsOn(ClientConnection *conn);
    void decSessionsOn(ClientConnection *conn);
    void sendOrInsertInbox(Packet *packet);
    void fetchInboxContent();
    vector<ClientConnection *> *getSessions();

private:
    string profile_id;
    int sessions_on;
    vector<Profile *> *followers;
    Inbox *inbox;
    vector<ClientConnection *> *sessions;
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
};

#endif
