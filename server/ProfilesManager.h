#ifndef TWITTER_PROFILES_MANAGER_H
#define TWITTER_PROFILES_MANAGER_H

#include <string>
#include <map>
#include <pthread.h>
#include "Profile.h"
#include "../commons/ClientConnection.h"
#include "../commons/Packet.h"
#include "../lib/json.hpp"

#define MAX_SESSIONS 2

using namespace std;
using json = nlohmann::json;

class ProfilesManager
{
public:
    ProfilesManager(string jsonFilename);
    bool insertProfile(string profileId, Profile *profile);
    Profile *getProfileById(string profileId);
    bool hasProfile(string profileId);
    void addFollowerTo(string followed, Profile *follower);
    void sendToFollowersOf(Profile *profile, Packet *packet);
    Profile *createProfileIfNotExists(string profileId, ClientConnection *conn);
    void sendCloseConnToAll();

private:
    string jsonFilename;
    map<string, Profile *> *profiles;
    void toJsonFile();
    void fromJsonFile();
};

#endif
