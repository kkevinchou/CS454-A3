#ifndef SENDER_H
#define SENDER_H

#include <string>

using namespace std;
class Sender
{
	private:
		int _sfd;
        void sendUnsignedInt(unsigned int i);
	public:
		Sender(int socketFileDescriptor);
		int sendMessage(string s);
};



#endif