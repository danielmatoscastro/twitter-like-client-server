#ifndef TWITTER_PROFILES_MANAGER_H
#define TWITTER_PROFILES_MANAGER_H

#include <string>
#include <map>
#include "Profile.h"
#include "../lib/json.hpp"

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

private:
    string jsonFilename;
    map<string, Profile *> *profiles;
    void toJsonFile();
};

#endif
