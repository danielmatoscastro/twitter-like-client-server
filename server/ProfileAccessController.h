#ifndef TWITTER_PROFILE_ACCESS_CONTROLLER_H
#define TWITTER_PROFILE_ACCESS_CONTROLLER_H

#include <string>
#include <map>
#include <pthread.h>
#include "Profile.h"
#include "ClientConnection.h"
#include "../commons/Packet.h"
#include "../lib/json.hpp"
#include "ProfilesManager.h"

using namespace std;

class ProfileAccessController
{
public:
    ProfileAccessController(string jsonFilename);
    bool insertProfile(string profileId, Profile *profile);
    Profile *getProfileById(string profileId);
    bool hasProfile(string profileId);
    void addFollowerTo(string followed, Profile *follower);
    void sendToFollowersOf(Profile *profile, Packet *packet);
    Profile *createProfileIfNotExists(string profileId, ClientConnection *conn);
private:
    ProfilesManager *profilesManager;
    int num_readers;
    int num_writers;
    void requestRead();
    void requestWrite();
    void releaseRead();
    void releaseWrite();
    pthread_cond_t ok_to_read;
    pthread_cond_t ok_to_write;
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
};

#endif
