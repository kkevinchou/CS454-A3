#include "sender.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include "helpers.h"
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "rwbuffer.h"
#include <cerrno>
static bool debug = false;
using namespace std;

int Sender::sendArray(unsigned int length, char data[])
{
	char * bytesPtr = data;
	unsigned int sendSize = length;
	int n;
	if(debug)
	{
		 for(unsigned int i = 0; i < length; i++)
		 {
		 	cout << (int)data[i] << " ";
		 }
		 cout << endl;
	}

	while(sendSize > 0)
	{
		// send the first 4 bytes
		// keep sending until send returns 0
		n = send(_sfd, bytesPtr, sendSize, 0);

		if (n == 0)
		{

			break;
		}
		else if(n<0)
		{
			cerr << "ERROR: send returned an error "<<errno<< endl;
			return n;
		}

		sendSize -= n;
		bytesPtr += n;
	}

	return sendSize;
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

 	return sendArray(messageSize + 8, buffer);
}

Sender::Sender(int socketFileDescriptor)
{
	_sfd = socketFileDescriptor;
}

/*****************************************************
=== PROTOCOL FUNCTIONS
*****************************************************/

int Sender::sendTerminateMessage() {


 	sendMessage(0, TERMINATE, NULL);
 	return 0;
}

int Sender::sendRegisterMessage(string serverID, unsigned short port, string name, int argTypes[]) {
	unsigned int serverIdSize = serverID.size() + 1;
	unsigned int portSize = 2;
	unsigned int nameSize = name.size() + 1;
	unsigned int argTypesLength = 0;
	while (argTypes[argTypesLength++]);

 	unsigned int messageSize = 4 + serverIdSize + portSize + nameSize + argTypesLength * 4;
 	char buffer[messageSize];
 	char *bufferP = buffer;

 	RWBuffer b;
 	bufferP = b.insertUnsignedIntToBuffer(serverID.size()+1, bufferP);
 	bufferP = b.insertStringToBuffer(serverID, bufferP);
 	bufferP = b.insertShortToBuffer(port, bufferP);
 	bufferP = b.insertStringToBuffer(name, bufferP);
 	bufferP = b.insertIntArrayToBuffer(argTypes, argTypesLength, bufferP);

 	return sendMessage(messageSize, REGISTER, buffer);
}

int Sender::sendRegisterSuccessMessage(ReasonCode code) {
	unsigned int reasonCodeSize = 4;

	unsigned int messageSize = reasonCodeSize;
	char buffer[messageSize];
	char *bufferP = buffer;

	RWBuffer b;
	bufferP = b.insertIntToBuffer(static_cast<int>(code), bufferP);

 	return sendMessage(messageSize, REGISTER_SUCCESS, buffer);
}

int Sender::sendRegisterFailureMessage(ReasonCode code) {
	unsigned int reasonCodeSize = 4;

	unsigned int messageSize = reasonCodeSize;
	char buffer[messageSize];
	char *bufferP = buffer;

	RWBuffer b;
	bufferP = b.insertIntToBuffer(static_cast<int>(code), bufferP);

 	return sendMessage(messageSize, REGISTER_FAILURE, buffer);
}

int Sender::sendLocRequestMessage(string name, int argTypes[]) {
	unsigned int nameSize = name.size() + 1;
	unsigned int argTypesLength = 0;
	while (argTypes[argTypesLength++]);

	unsigned int messageSize = 4+nameSize + argTypesLength * 4;
	char buffer[messageSize];
	char *bufferP = buffer;

	RWBuffer b;
	bufferP = b.insertUnsignedIntToBuffer(name.size()+1, bufferP);
	bufferP = b.insertStringToBuffer(name, bufferP);
 	bufferP = b.insertIntArrayToBuffer(argTypes, argTypesLength, bufferP);

 	return sendMessage(messageSize, LOC_REQUEST, buffer);
}

int Sender::sendLocSuccessMessage(string serverID, unsigned short port) {
	unsigned int serverIdSize = serverID.size() + 1;
	unsigned int portSize = 2;

	unsigned int messageSize = 4 + serverIdSize + portSize;
	char buffer[messageSize];
	char *bufferP = buffer;

	RWBuffer b;
	bufferP = b.insertUnsignedIntToBuffer(serverID.size()+1, bufferP);
	bufferP = b.insertStringToBuffer(serverID, bufferP);
 	bufferP = b.insertShortToBuffer(port, bufferP);

 	return sendMessage(messageSize, LOC_SUCCESS, buffer);

}

int Sender::sendLocCacheSuccessMessage(list<service_info*>&l)
{
	unsigned int portSize = 2;

	unsigned int messageSize = 0;
	unsigned int numLocs = 0;
	list<service_info*>::iterator it = l.begin();
	while(it != l.end())
	{
		service_info * s = *it;
		messageSize += 4 + s->server_identifier.size() + 1 + portSize;
		it++;
		numLocs++;
	}

	char buffer[messageSize];
	char *bufferP = buffer;

	RWBuffer b;
	it = l.begin();
	for(int i = 0; i < numLocs; i++)
	{
		service_info * s = *it;
		bufferP = b.insertUnsignedIntToBuffer(s->server_identifier.size()+1, bufferP);
		bufferP = b.insertCharArrayToBuffer(s->server_identifier.c_str(), s->server_identifier.size()+1,bufferP);
	 	bufferP = b.insertShortToBuffer(s->port, bufferP);

	 	it++;
	}

 	return sendMessage(messageSize, LOC_CACHE_SUCCESS, buffer);
}

int Sender::sendLocCacheRequestMessage(string functionName, int*argTypes)
{
	RWBuffer b;
	unsigned int argsLength = b.returnArgTypesLength(argTypes);

	unsigned int messageSize = 4 + functionName.size()+1 + argsLength*4;


	char buffer[messageSize];
	char * bufferP = buffer;
	bufferP = b.insertUnsignedIntToBuffer(functionName.size()+1, bufferP);
	bufferP = b.insertCharArrayToBuffer(functionName.c_str(), functionName.size()+1, bufferP);
	bufferP = b.insertIntArrayToBuffer(argTypes, argsLength, bufferP);
	return sendMessage(messageSize, LOC_CACHE_REQUEST, buffer);

}
int Sender::sendLocCacheFailureMessage(ReasonCode reasonCode) 
{
		unsigned int reasonCodeSize = 4;

	unsigned int messageSize = reasonCodeSize;
	char buffer[messageSize];
	char *bufferP = buffer;

	RWBuffer b;
	bufferP = b.insertIntToBuffer(static_cast<int>(reasonCode), bufferP);

 	return sendMessage(messageSize, LOC_CACHE_FAILURE, buffer);
}
int Sender::sendLocFailureMessage(ReasonCode reasonCode) {
	unsigned int reasonCodeSize = 4;

	unsigned int messageSize = reasonCodeSize;
	char buffer[messageSize];
	char *bufferP = buffer;

	RWBuffer b;
	bufferP = b.insertIntToBuffer(static_cast<int>(reasonCode), bufferP);

 	return sendMessage(messageSize, LOC_FAILURE, buffer);
}