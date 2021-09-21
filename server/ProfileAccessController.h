#ifndef TWITTER_PROFILE_ACCESS_CONTROLLER_H
#define TWITTER_PROFILE_ACCESS_CONTROLLER_H

#include <string>
#include <pthread.h>

using namespace std;

class ProfileAccessController
{
public:
    ProfileAccessController();
    void requestRead();
    void requestWrite();
    void releaseRead();
    void releaseWrite();

private:
    int num_readers;
    int num_writers;
    pthread_cond_t ok_to_read;
    pthread_cond_t ok_to_write;
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
};

#endif
