#include <fstream>
#include <iostream>
#include "ProfilesManager.h"

using namespace std;

ProfilesManager::ProfilesManager(string jsonFilename)
{
    this->jsonFilename = jsonFilename;
    this->profiles = new map<string, Profile *>();
    this->controller = new ProfileAccessController();
}

bool ProfilesManager::insertProfile(string profileId, Profile *profile)
{
    controller->requestWrite();

    bool isInMap = profiles->count(profileId) > 0;
    if (!isInMap)
    {
        profiles->insert(pair<string, Profile *>(profileId, profile));
    }

    this->toJsonFile();

    controller->releaseWrite();

    return !isInMap;
}
Profile *ProfilesManager::getProfileById(string profileId)
{
    cout << "vou pegar o segundo lock" << endl;
    controller->requestRead();
    cout << "peguei o segundo lock" << endl;
    auto it = profiles->find(profileId);

    if (it == profiles->end())
    {
        controller->releaseRead();
        return nullptr;
    }

    controller->releaseRead();
    return it->second;
}

bool ProfilesManager::hasProfile(string profileId)
{
    controller->requestRead();
    bool has = profiles->count(profileId) > 0;
    controller->releaseRead();

    return has;
}

void ProfilesManager::addFollowerTo(string followed, Profile *follower)
{
    controller->requestWrite();

    if (follower->getProfileId() == followed)
    {
        controller->releaseWrite();
        return;
    }

    cout << follower->getProfileId() << " wants to follow " << followed << endl;

    Profile *profileToFollow = this->getProfileById(followed);
    if (profileToFollow == nullptr)
    {
        controller->releaseWrite();
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

    controller->releaseWrite();
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

void ProfilesManager::sendToFollowersOf(Profile *profile, Packet *packet)
{
    controller->requestWrite();

    for (int i = 0; i < profile->getFollowers()->size(); i++)
    {
        Profile *follower = profile->getFollowers()->at(i);
        cout << "inserindo " << packet->getPayload() << " na inbox de " << follower->getProfileId() << endl;
        follower->sendOrInsertInbox(packet);
    }
    cout << profile->getProfileId() << " says: " << packet->getPayload() << endl;

    controller->releaseWrite();
}

Profile *ProfilesManager::createProfileIfNotExists(string profileId, ClientConnection *conn)
{
    controller->requestWrite();

    cout << "peguei o lock" << endl;
    Profile *profile = this->getProfileById(profileId);
    cout << "fiz o get" << endl;
    if (profile != nullptr)
    {
        if (profile->getSessionsOn() == MAX_SESSIONS)
        {
            Packet *toClient = new Packet(CmdType::CLOSE_CONN);
            conn->sendPacket(toClient);
            conn->close();
            controller->releaseWrite();

            pthread_exit(nullptr);
        }
    }
    else
    {
        profile = new Profile(profileId);
        this->insertProfile(profileId, profile);
        cout << "PROFILE " << profileId << " has been planted" << endl;
    }
    //Incluir mutex fim

    profile->incSessionsOn(conn);

    controller->releaseWrite();

    return profile;
}
