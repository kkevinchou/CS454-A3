#ifndef SENDER_H
#define SENDER_H

#include <string>

using namespace std;
class Sender
{
	private:
		int _sfd;
	public:
		Sender(int socketFileDescriptor);
		int sendMessage(string s);
        int sendUnsignedInt(unsigned int i);
};



#endif