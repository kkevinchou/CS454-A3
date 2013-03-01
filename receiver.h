#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>
using namespace std;
class Receiver
{
	private:
		int _sfd;
	public:
		Receiver(int socketFileDescriptor);
		string receiveMessage(); // THIS BLOCKS UNTIL A FULL MESSAGE IS RECEIVED. Returns NULL on failure
		string receiveMessageGivenSize(unsigned int messageSize);
		unsigned int receiveMessageSize();


};
#endif