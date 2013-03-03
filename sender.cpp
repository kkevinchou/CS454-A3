#include "sender.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include "helpers.h"
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "rwbuffer.h"
static bool debug = false;
using namespace std;

int Sender::sendArray(unsigned int length, char data[])
{
	char * bytesPtr = data;
	unsigned int sentSize = length;
	int n;
	if(debug)
	{
		cout << "Sending size "<<length<<endl;
		 cout << "Sending: ";
		 for(unsigned int i = 0; i < length; i++)
		 {
		 	cout << (int)data[i] << " ";
		 }
		 cout << endl;
	}

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


int Sender::getSocketFD()
{
	return _sfd;
}

int Sender::sendUnsignedInt(unsigned int i) {
	char bytes[4];
	RWBuffer b;
	b.insertIntToBuffer(i, bytes);

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

int Sender::sendMessage(unsigned int messageSize, MessageType msgType, char message[]) {
	char buffer[messageSize + 8];
	char *bufferP = buffer;

	RWBuffer b;
	bufferP = b.insertUnsignedIntToBuffer(messageSize, bufferP);
 	bufferP = b.insertIntToBuffer(static_cast<int>(msgType), bufferP);

 	for (unsigned int i = 0; i < messageSize; i ++) {
 		// Two bytes for message size and message type
 		buffer[i + 8] = message[i];
 	}

 	sendArray(messageSize + 8, buffer);
}

Sender::Sender(int socketFileDescriptor)
{
	_sfd = socketFileDescriptor;
}

/*****************************************************
=== PROTOCOL FUNCTIONS
*****************************************************/

int Sender::sendRegisterMessage(string serverID, short port, string name, int argTypes[])
{
	int serverIdSize = serverID.size() + 1;
	int portSize = 2;
	int nameSize = name.size() + 1;
	int argTypesLength = 0;
	while (argTypes[argTypesLength++]);

 	unsigned int messageSize = serverIdSize + portSize + nameSize + argTypesLength * 4;
 	char buffer[messageSize];
 	char *bufferP = buffer;

 	RWBuffer b;
 	bufferP = b.insertStringToBuffer(serverID, bufferP);
 	bufferP = b.insertShortToBuffer(port, bufferP);
 	bufferP = b.insertStringToBuffer(name, bufferP);
 	bufferP = b.insertIntArrayToBuffer(argTypes, argTypesLength, bufferP);

 	sendMessage(messageSize, REGISTER, buffer);

 	return 0;
}

int Sender::sendLocRequestMessage(string name, int argTypes[]) {
	int nameSize = name.size() + 1;
	int argTypesLength = 0;
	while (argTypes[argTypesLength++]);

	unsigned int messageSize = nameSize + argTypesLength * 4;
	char buffer[messageSize];
	char *bufferP = buffer;

	RWBuffer b;
	bufferP = b.insertStringToBuffer(name, bufferP);
 	bufferP = b.insertIntArrayToBuffer(argTypes, argTypesLength, bufferP);

 	sendMessage(messageSize, LOC_REQUEST, buffer);

 	return 0;
}