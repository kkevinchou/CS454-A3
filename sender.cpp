#include "sender.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include "helpers.h"

using namespace std;

void Sender::sendUnsignedInt(unsigned int i)
{
	unsigned char sizeBytes[4];
	unsigned char * sizeBytesP = sizeBytes;

	sizeBytes[0] = (i >> 24) & 0xFF;
	sizeBytes[1] = (i >> 16) & 0xFF;
	sizeBytes[2] = (i >> 8) & 0xFF;
	sizeBytes[3] = i & 0xFF;

	int sizeSize = 4;

	while(true)
	{
		//send the first 4 bytes
		// keep sending until send returns 0
		n = send(_sfd, sizeBytesP, sizeSize, 0);
		if (n == 0)
		{
			break;
		}
        else if (n < 0)
        {
            // error("ERROR writing to socket for message: ");
            return -1;
		}

		sizeSize -= n;
		sizeBytesP += n;

		sleep(2);
	}
}

int Sender::sendMessage(string s)
{
	int n;

	unsigned int size = s.size()+1;
	const char * cstr = s.c_str();

	sendUnsignedInt(size);

	while(true)
	{
		// send the message
		// keep sending until send returns 0
		n = send(_sfd,cstr, size, 0);
		if(n==0)
		{
			break;
		}
        else if (n < 0)
        {
            // error("ERROR writing to socket for message: ");
            return -1;
		}

		size -= n;
		cstr += n;

		if(size == 0) break;

		sleep(2);
	}
	return 0;
}

Sender::Sender(int socketFileDescriptor)
{
	_sfd = socketFileDescriptor;
}