#ifndef SENDER_H
#define SENDER_H

#include <string>
#include "constants.h"

using namespace std;
class Sender
{
	private:
		int _sfd;

	public:
		Sender(int socketFileDescriptor);
		int sendMessage(string s);
                int sendUnsignedInt(unsigned int i);
                int sendArray(unsigned int length, char data[]);

               
                int sendRegisterMessage(string serverID, short port, string name, int argTypesLength, int argTypes[]);
                int sendLocRequestMessage(string name, int argTypes[]);

                int getSocketFD();
};

#endif