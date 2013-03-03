#include "sender.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include "helpers.h"
#include <stdio.h>
#include <string.h>
#include "constants.h"

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
char *Sender::addCStringToBuffer(char * c, char *bufferP )
{
	while(*c != '\0')
	{
		*bufferP = *c;
		bufferP++;
		c++;
	}
	*bufferP = *c;
	bufferP++;
	return bufferP;
}
char * Sender::addStringToBuffer(string s, char *bufferP)
{
	unsigned int i = 0;
	for (i = 0; i < s.size(); i++) {
		bufferP[i] = s[i];
	}
	bufferP[i] = '\0';

	return bufferP + i + 1;
}

char * Sender::addIntToBuffer(int i, char *bufferP)
{
	bufferP[0] = (i >> 24) & 0xFF;
	bufferP[1] = (i >> 16) & 0xFF;
	bufferP[2] = (i >> 8) & 0xFF;
	bufferP[3] = i & 0xFF;

	return bufferP + 4;
}

char * Sender::addUnsignedIntToBuffer(unsigned int u, char *bufferP)
{
	bufferP[0] = (u >> 24) & 0xFF;
	bufferP[1] = (u >> 16) & 0xFF;
	bufferP[2] = (u >> 8) & 0xFF;
	bufferP[3] = u & 0xFF;

	return bufferP + 4;
}

char * Sender::addShortToBuffer(short s, char *bufferP)
{
	bufferP[0] = (s >> 8) & 0xFF;
	bufferP[1] = s & 0xFF;

	return bufferP + 2;
}

char * Sender::addIntBufferToBuffer(int intBuf[], int numInts, char *bufferP) {
	char *ptr = bufferP;
	for (int i = 0; i < numInts; i++) {
		ptr = addIntToBuffer(intBuf[i], ptr);
	}

	return ptr;
}

char *addCharToBuffer(char c, char *bufferP) {
	bufferP[0] = c;
	return bufferP + 1;
}

// char *addDoubleToBuffer(double d, char *bufferP) {
// 	int size = sizeof(d);
// 	for (int i = 1; i <= size; i++) {
// 		bufferP[i - 1] = (d >> ((size - i) * 8)) & 0xFF;
// 	}
// 	return bufferP + size;
// }

// char *addFloatToBuffer(float f, char *bufferP) {
// 	int size = sizeof(f);
// 	for (int i = 1; i <= size; i++) {
// 		bufferP[i - 1] = (f >> ((size - i) * 8)) & 0xFF;
// 	}
// 	return bufferP + size;
// }

char *addLongToBuffer(long l, char *bufferP) {
	int size = sizeof(l);
	for (int i = 1; i <= size; i++) {
		bufferP[i - 1] = (l >> ((size - i) * 8)) & 0xFF;
	}
	return bufferP + size;
}

int Sender::sendRegisterMessage(string serverID, short port, string name, int argTypesLength, int argTypes[])
{
	int serverIdSize = serverID.size() + 1;
	int portSize = 2;
	int nameSize = name.size() + 1;
	int argTypesSize = argTypesLength;
	// TODO: calculate argTypesLength in here rather than accept as a parameter

 	unsigned int messageSize = serverIdSize + portSize + nameSize + argTypesSize*4;
 	char buffer[messageSize+8];
 	char *bufferP = buffer;

 	bufferP = addUnsignedIntToBuffer(messageSize, bufferP);
 	bufferP = addIntToBuffer(static_cast<int>(REGISTER), bufferP);
 	bufferP = addStringToBuffer(serverID, bufferP);
 	bufferP = addShortToBuffer(port, bufferP);
 	bufferP = addStringToBuffer(name, bufferP);
 	bufferP = addIntBufferToBuffer(argTypes, argTypesLength, bufferP);

 	sendArray(messageSize + 8, buffer);

 	return 0;
}

int Sender::sendLocRequestMessage(string name, int argTypes[]) {
	int nameSize = name.size() + 1;
	int argTypesLength = 0;

	while (argTypes[argTypesLength++]);

	unsigned int messageSize = nameSize + argTypesLength * 4;
	char buffer[messageSize+8];
	char *bufferP = buffer;

	bufferP = addUnsignedIntToBuffer(messageSize, bufferP);
 	bufferP = addIntToBuffer(static_cast<int>(LOC_REQUEST), bufferP);
	bufferP = addStringToBuffer(name, bufferP);
 	bufferP = addIntBufferToBuffer(argTypes, argTypesLength, bufferP);

 	sendArray(messageSize + 8, buffer);

 	return 0;
}

int Sender::sendUnsignedInt(unsigned int i) {
	char bytes[4];

	bytes[0] = (i >> 24) & 0xFF;
	bytes[1] = (i >> 16) & 0xFF;
	bytes[2] = (i >> 8) & 0xFF;
	bytes[3] = i & 0xFF;

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