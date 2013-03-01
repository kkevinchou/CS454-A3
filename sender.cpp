#include "sender.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include "helpers.h"

using namespace std;



void Sender::sendMessage(string s)
{
	int n;

	unsigned int size = s.size()+1;
	const char * cstr = s.c_str();

	unsigned char sizeBytes[4];
	unsigned char * sizeBytesP = sizeBytes;

	sizeBytes[0] = (size >> 24) & 0xFF;
	sizeBytes[1] = (size >> 16) & 0xFF;
	sizeBytes[2] = (size >> 8) & 0xFF;
	sizeBytes[3] = size & 0xFF;

	int sizeSize = 4;

	while(true)
	{
		//send the first 4 bytes
		// keep sending until send returns 0
		n = send(_sfd,sizeBytesP, sizeSize, 0);
		if(n==0)
		{
			break;
		}
        else if (n < 0)
        {
            error("ERROR writing to socket for message: ");
		}

		sizeSize -= n;
		sizeBytesP += n;

		sleep(2);
	}

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
            error("ERROR writing to socket for message: ");
		}


		size -= n;
		cstr += n;

		if(size == 0) break;

		sleep(2);
	}
		        
}

Sender::Sender(int socketFileDescriptor)
{
	_sfd = socketFileDescriptor;
}