#include "sender.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include "helpers.h"
#include <stdio.h>
#include <string.h>
#include "constants.h"

using namespace std;
 int Sender::sendArray(unsigned int length, char data[])
 {
	 char * bytesPtr = data;
	 unsigned int sentSize = length;
	 int n;
	/* cout << "Sending size "<<length<<endl;
	 cout << "Sending: ";
	 for(int i = 0; i < length; i++)
	 {
	 	cout << data[i] << " ";
	 }
	 cout << endl;*/
	while(true)
	{
		// send the first 4 bytes
		// keep sending until send returns 0
		n = send(_sfd, bytesPtr, sentSize, 0);

		if (n == 0)
		{
			break;
		}
		else if(n<0)
		{
			return -1;
		}

		sentSize -= n;
		bytesPtr += n;

		sleep(2);
	}

	return sentSize;
 }

int Sender::sendUnsignedInt(unsigned int i) {
	 char bytes[4];
	 char * bytesPtr = bytes;

	bytes[0] = (i >> 24) & 0xFF;
	bytes[1] = (i >> 16) & 0xFF;
	bytes[2] = (i >> 8) & 0xFF;
	bytes[3] = i & 0xFF;

	int size = 4;
	int numSentBytes = 0;

	return sendArray(4, bytes);
	
}

int Sender::sendMessage(string s)
{
	unsigned int size = s.size()+1;
 	sendUnsignedInt(size);
	char cstr[size];
	strcpy(cstr, s.c_str());

	return sendArray(size, cstr);
}

Sender::Sender(int socketFileDescriptor)
{
	_sfd = socketFileDescriptor;
}