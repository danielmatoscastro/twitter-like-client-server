#ifndef TWITTER_PROFILES_MANAGER_H
#define TWITTER_PROFILES_MANAGER_H

#include <string>
#include <map>
#include "Profile.h"

using namespace std;

class ProfilesManager
{
public:
    ProfilesManager();
    bool insertProfile(string profileId, Profile *profile);
    Profile *getProfileById(string profileId);
    bool hasProfile(string profileId);

private:
    map<string, Profile *> *profiles;
};

#endif
