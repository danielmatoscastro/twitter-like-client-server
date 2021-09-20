
#include "ProfilesManager.h"

using namespace std;

ProfilesManager::ProfilesManager()
{
    this->profiles = new map<string, Profile *>();
}

bool ProfilesManager::insertProfile(string profileId, Profile *profile)
{
    bool isInMap = profiles->count(profileId) > 0;
    if (!isInMap)
    {
        profiles->insert(pair<string, Profile *>(profileId, profile));
    }

    return !isInMap;
}
Profile *ProfilesManager::getProfileById(string profileId)
{
    auto it = profiles->find(profileId);

    if (it == profiles->end())
    {
        return nullptr;
    }

    return it->second;
}

bool ProfilesManager::hasProfile(string profileId)
{
    return profiles->count(profileId) > 0;
}
