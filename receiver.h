#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>
#include "constants.h"
using namespace std;
class Receiver
{
	private:
		int _sfd;
	public:
		Receiver(int socketFileDescriptor);
		string receiveMessageAsString(); // THIS BLOCKS UNTIL A FULL MESSAGE IS RECEIVED. Returns NULL on failure
		int receiveMessageGivenSize(unsigned int messageSize, char ret[]); // returns a message char * of length MessageSize
		int receiveMessageSize(); // returns a positive int (4 bytes)
		unsigned int convertToUnsignedInt(char d[4]);
		int convertToInt(char d[4]);
		MessageType receiveMessageType();

		int receiveRegisterMessage(string &serverID, int &port, string &name, int argTypes[]);

};
#endif