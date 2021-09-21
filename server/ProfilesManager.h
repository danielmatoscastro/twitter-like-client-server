#ifndef TWITTER_PROFILES_MANAGER_H
#define TWITTER_PROFILES_MANAGER_H

#include <string>
#include <map>
#include <pthread.h>
#include "Profile.h"
#include "ClientConnection.h"
#include "../commons/Packet.h"
#include "../lib/json.hpp"
#include "ProfileAccessController.h"

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

private:
    string jsonFilename;
    map<string, Profile *> *profiles;
    ProfileAccessController *controller;
    void toJsonFile();
};

#endif
