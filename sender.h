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
        int sendMessage(unsigned int messageSize, MessageType msgType, char message[]);
        int sendUnsignedInt(unsigned int i);
        int sendArray(unsigned int length, char data[]);
        int getSocketFD();

        // Protocol Functions
        int sendTerminateMessage();
        int sendRegisterMessage(string serverID, unsigned short port, string name, int argTypes[]);
        int sendLocRequestMessage(string name, int argTypes[]);
        int sendLocSuccessMessage(string serverID, unsigned short port);
        int sendLocFailureMessage(ReasonCode reasonCode);
};

#endif