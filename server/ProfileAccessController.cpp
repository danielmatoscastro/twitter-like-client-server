
#include "ProfileAccessController.h"

using namespace std;

ProfileAccessController::ProfileAccessController()
{
    this->num_readers = 0;
    this->num_writers = 0;
    pthread_cond_init(&ok_to_read, NULL);
    pthread_cond_init(&ok_to_write, NULL);
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
