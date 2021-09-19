#ifndef TWITTER_PROFILE_H
#define TWITTER_PROFILE_H

#include <string>
#include <vector>
#include "../commons/Packet.h"

using namespace std;

class Profile
{
public:
    Profile(string profile_id);
    string getProfileId();
    vector<Packet *> *getMessages();
    int getSessionsOn();
    vector<Profile *> *getFollowers();
    vector<Packet *> *getInbox();
    void incSessionsOn();

private:
    string profile_id;
    vector<Packet *> *messages;
    int sessions_on;
    vector<Profile *> *followers;
    vector<Packet *> *inbox;
};

#endif
