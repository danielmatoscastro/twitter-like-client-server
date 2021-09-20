#include <fstream>
#include <iostream>
#include "ProfilesManager.h"

using namespace std;

ProfilesManager::ProfilesManager(string jsonFilename)
{
    this->jsonFilename = jsonFilename;
    this->profiles = new map<string, Profile *>();
}

bool ProfilesManager::insertProfile(string profileId, Profile *profile)
{
    bool isInMap = profiles->count(profileId) > 0;
    if (!isInMap)
    {
        profiles->insert(pair<string, Profile *>(profileId, profile));
    }

    this->toJsonFile();

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

void ProfilesManager::addFollowerTo(string followed, Profile *follower)
{
    Profile *profileToFollow = this->getProfileById(followed);
    auto it = find_if(
        profileToFollow->getFollowers()->begin(),
        profileToFollow->getFollowers()->end(),
        [&](Profile * p) {return p->getProfileId() == follower->getProfileId();}
    );
    
    if (it == profileToFollow->getFollowers()->end()){
        profileToFollow->getFollowers()->push_back(follower);
        this->toJsonFile();
    }
}

void ProfilesManager::toJsonFile()
{
    json j;

    for (auto entry : *this->profiles)
    {
        auto names = new vector<string>();
        for (auto follower : *entry.second->getFollowers())
        {
            names->push_back(follower->getProfileId());
        }
        j[entry.first] = *names;
    }

    ofstream jsonFile(this->jsonFilename, ios::out);
    jsonFile << j;
    jsonFile.close();

    cout << j << endl;
}
