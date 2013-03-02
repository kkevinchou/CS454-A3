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
        int sendArray(unsigned int length, char data[]);

        char * addShortToBuffer(short s, char *bufferP);
        char * addIntToBuffer(int i, char *bufferP);
        char * addStringToBuffer(string s, char *bufferP);
        char * addIntBufferToBuffer(int intBuf[], int numInts, char *bufferP);

        int sendRegisterMessage(string serverID, short port, string name, int argTypesLength, int argTypes[]);
};

#endif