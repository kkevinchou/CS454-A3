#include "sender.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include "helpers.h"

using namespace std;

int Sender::sendUnsignedInt(unsigned int i) {
	unsigned char bytes[4];
	unsigned char * bytesPtr = bytes;

	bytes[0] = (i >> 24) & 0xFF;
	bytes[1] = (i >> 16) & 0xFF;
	bytes[2] = (i >> 8) & 0xFF;
	bytes[3] = i & 0xFF;

	int size = 4;
	int numSentBytes = 0;

	while(true)
	{
		// send the first 4 bytes
		// keep sending until send returns 0
		numSentBytes = send(_sfd, bytesPtr, size, 0);

		if (numSentBytes == 0 || numSentBytes < 0)
		{
			break;
		}

		size -= numSentBytes;
		bytesPtr += numSentBytes;

		sleep(2);
	}

	return numSentBytes;
}

int Sender::sendMessage(string s)
{
	int sentBytes;

	unsigned int size = s.size()+1;
	const char * cstr = s.c_str();

	sentBytes = sendUnsignedInt(size);

	if (sentBytes < 0)
	{
		return sentBytes;
	}

	while(true)
	{
		// send the message
		// keep sending until send returns 0
		sentBytes = send(_sfd, cstr, size, 0);

		if(sentBytes == 0 || sentBytes < 0)
		{
			break;
		}

		size -= sentBytes;
		cstr += sentBytes;

		if(size == 0) break;

		sleep(2);
	}
	return sentBytes;
}

Sender::Sender(int socketFileDescriptor)
{
	_sfd = socketFileDescriptor;
}