
#include "ProfileAccessController.h"

using namespace std;

ProfileAccessController::ProfileAccessController(string jsonFilename)
{
    this->profilesManager = new ProfilesManager(jsonFilename);
    this->num_readers = 0;
    this->num_writers = 0;
    pthread_cond_init(&ok_to_read, NULL);
    pthread_cond_init(&ok_to_write, NULL);
}

bool ProfileAccessController::insertProfile(string profileId, Profile *profile)
{
    this->requestWrite();
    bool rtn = this->profilesManager->insertProfile(profileId, profile);
    this->releaseWrite();
    return rtn;
}

Profile *ProfileAccessController::getProfileById(string profileId)
{
    this->requestRead();
    Profile *profile = this->profilesManager->getProfileById(profileId);
    this->releaseRead();
    return profile;
}

bool ProfileAccessController::hasProfile(string profileId)
{
    this->requestRead();
    bool rtn = this->profilesManager->hasProfile(profileId);
    this->releaseRead();
    return rtn;
}

void ProfileAccessController::addFollowerTo(string followed, Profile *follower)
{
    this->requestWrite();
    this->profilesManager->addFollowerTo(followed, follower);
    this->releaseWrite();
}

void ProfileAccessController::sendToFollowersOf(Profile *profile, Packet *packet)
{
    this->requestWrite();
    this->profilesManager->sendToFollowersOf(profile, packet);
    this->releaseWrite();
}

Profile *ProfileAccessController::createProfileIfNotExists(string profileId, ClientConnection *conn)
{
    this->requestWrite();
    Profile *profile = this->profilesManager->createProfileIfNotExists(profileId, conn);
    this->releaseWrite();
    return profile;
}

void ProfileAccessController::sendCloseConnToAll()
{
    this->requestWrite();
    this->profilesManager->sendCloseConnToAll();
    this->releaseWrite();
}

void ProfileAccessController::requestRead()
{
    pthread_mutex_lock(&m);

    while (num_writers > 0)
    {
        pthread_cond_wait(&ok_to_read, &m);
    }

    num_readers = num_readers + 1;

    pthread_mutex_unlock(&m);
}

void ProfileAccessController::requestWrite()
{
    pthread_mutex_lock(&m);

    while (num_readers > 0 || num_writers > 0)
    {
        pthread_cond_wait(&ok_to_write, &m);
    }
    num_writers = num_writers + 1;

    pthread_mutex_unlock(&m);
}

void ProfileAccessController::releaseRead()
{
    pthread_mutex_lock(&m);

    num_readers = num_readers - 1;
    if (num_readers == 0)
    {
        pthread_cond_signal(&ok_to_write);
    }

    pthread_mutex_unlock(&m);
}

void ProfileAccessController::releaseWrite()
{
    pthread_mutex_lock(&m);

    num_writers = num_writers - 1;
    pthread_cond_signal(&ok_to_write);
    pthread_cond_broadcast(&ok_to_read);

    pthread_mutex_unlock(&m);
}
