#include <fstream>
#include <iostream>
#include "ProfilesManager.h"
#include <experimental/filesystem>

using namespace std;

ProfilesManager::ProfilesManager(string jsonFilename)
{
    this->jsonFilename = jsonFilename;
    this->profiles = new map<string, Profile *>();
    //this->fromJsonFile();
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
    bool has = profiles->count(profileId) > 0;

    return has;
}

void ProfilesManager::addFollowerTo(string followed, Profile *follower)
{
    if (follower->getProfileId() == followed)
    {
        return;
    }

    cout << follower->getProfileId() << " wants to follow " << followed << endl;

    Profile *profileToFollow = this->getProfileById(followed);
    if (profileToFollow == nullptr)
    {

        return;
    }

    auto it = find_if(
        profileToFollow->getFollowers()->begin(),
        profileToFollow->getFollowers()->end(),
        [&](Profile *p)
        { return p->getProfileId() == follower->getProfileId(); });

    if (it == profileToFollow->getFollowers()->end())
    {
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

    // Print json file
    // cout << j << endl;
}

void ProfilesManager::fromJsonFile()
{

    ifstream jsonFile(this->jsonFilename, ios::in);

    if (jsonFile)
    {
        json jDict;
        jsonFile >> jDict;
        jsonFile.close();

        for (auto &profile : jDict.items())
        {
            profiles->insert(pair<string, Profile *>(profile.key(), new Profile(profile.key())));
        }

        for (auto &profile : jDict.items())
        {
            for (auto follower : profile.value())
            {
                this->addFollowerTo(profile.key(), this->getProfileById(follower));
            }
        }
    }
}

void ProfilesManager::sendToFollowersOf(Profile *profile, Packet *packet)
{
    for (int i = 0; i < profile->getFollowers()->size(); i++)
    {
        Profile *follower = profile->getFollowers()->at(i);
        cout << "Adding \"" << packet->getPayload() << "\" in the inbox of " << follower->getProfileId() << endl;
        follower->sendOrInsertInbox(packet);
    }

    cout << profile->getProfileId() << " says: " << packet->getPayload() << endl;
}

Profile *ProfilesManager::createProfileIfNotExists(string profileId, ClientConnection *conn)
{
    cout << "createProfileIfNotExists" << endl;
    Profile *profile = this->getProfileById(profileId);
    if (profile != nullptr)
    {
        if (profile->getSessionsOn() == MAX_SESSIONS)
        {
            Packet *toClient = new Packet(CmdType::CLOSE_CONN);
            conn->sendPacket(toClient);
            conn->close();

            pthread_exit(nullptr);
        }
    }
    else
    {
        profile = new Profile(profileId);
        this->insertProfile(profileId, profile);
        cout << "Profile " << profileId << " was created." << endl;
    }

    profile->incSessionsOn(conn);

    return profile;
}

void ProfilesManager::sendCloseConnToAll()
{
    for (auto const &p : *this->profiles)
    {
        Profile *profile = p.second;
        for (ClientConnection *conn : *profile->getSessions())
        {
            conn->sendPacket(new Packet(CmdType::CLOSE_CONN));
            conn->close();
        }
    }
}
