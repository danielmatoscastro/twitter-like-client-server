#include "Profile.h"
#include <iostream>
#include <algorithm>

using namespace std;

Profile::Profile(string profile_id)
{
    this->profile_id = profile_id;
    this->sessions_on = 0;
    this->followers = new vector<Profile *>();
    this->inbox = new Inbox();
    this->sessions = new vector<ClientConnection *>();
}

string Profile::getProfileId()
{
    return this->profile_id;
}

int Profile::getSessionsOn()
{
    return this->sessions_on;
}

vector<Profile *> *Profile::getFollowers()
{
    return this->followers;
}

void Profile::incSessionsOn(ClientConnection *conn)
{
    pthread_mutex_lock(&m);
    this->sessions_on++;
    this->sessions->push_back(conn);
    cout << "Quantos tem? " << this->sessions->size() << endl;
    pthread_mutex_unlock(&m);
}

void Profile::decSessionsOn(ClientConnection *conn)
{
    pthread_mutex_lock(&m);
    auto it = find(this->sessions->begin(), this->sessions->end(), conn);
    if (it != this->sessions->end())
    {
        this->sessions->erase(it);
        this->sessions_on--;
        cout << "Decrementou" << endl;
    }
    pthread_mutex_unlock(&m);
}

void Profile::addFollower(Profile *follower)
{
    pthread_mutex_lock(&m);
    for (auto p : *followers)
    {
        if (follower->getProfileId() == p->getProfileId())
        {
            cout << "follower ja existe" << endl;
            return;
        }
    }
    cout << "Will add follower " << follower->getProfileId() << endl;

    this->followers->push_back(follower);
    pthread_mutex_unlock(&m);
}

void Profile::sendOrInsertInbox(Packet *packet)
{
    pthread_mutex_lock(&m);
    if (this->getSessionsOn() > 0)
    {
        cout << "entrou if" << endl;
        for (auto session : *this->sessions)
        {
            cout << "sending packet" << endl;
            session->sendPacket(packet);
        }
    }
    else
    {
        cout << "entrou else" << endl;
        this->inbox->insertPacket(packet);
    }
    pthread_mutex_unlock(&m);
}

void Profile::fetchInboxContent()
{
    pthread_mutex_lock(&m);
    while (this->inbox->hasPacket())
    {
        Packet *packet = this->inbox->popPacket();
        for (auto session : *this->sessions)
        {
            //send pending packet to client
            session->sendPacket(packet);
        }
    }
    pthread_mutex_unlock(&m);
}
