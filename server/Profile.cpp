#include "Profile.h"

using namespace std;

Profile::Profile(string profile_id)
{
    this->profile_id = profile_id;
    this->messages = new vector<Packet *>();
    this->sessions_on = 1;
    this->followers = new vector<Profile *>();
    this->inbox = new vector<Packet *>();
}

string Profile::getProfileId()
{
    return this->profile_id;
}

vector<Packet *> *Profile::getMessages()
{
    return this->messages;
}

int Profile::getSessionsOn()
{
    return this->sessions_on;
}

vector<Profile *> *Profile::getFollowers()
{
    return this->followers;
}

vector<Packet *> *Profile::getInbox()
{
    return this->inbox;
}

void Profile::incSessionsOn()
{
    this->sessions_on++;
}
