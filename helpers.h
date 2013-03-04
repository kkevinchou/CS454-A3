#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include "constants.h"
/*
return values:
0 - success
-1 - error while using library functions, check errno
-2 - error while opening socket
-3 - no such host
-4 - error while connecting
*/

using namespace std;

void error(string msg);
int setupSocketAndReturnDescriptor(char * serverAddressString, char * serverPortString);

int createSocket();

// Server Helpers

unsigned int sizeOfType(int type);

void listenOnSocket(int localSocketFd);
int acceptConnection(int localSocketFd);
MessageType getMessageTypeFromInt(int i);
unsigned short getPort(int localSocketFd);
string getHostname();

void printSettings(int localSocketFd);

int extractArgumentsMessage(char * bufferPointer, int argTypes[], void * args[], unsigned int argTypesLength);
unsigned int getClientServerMessageLength(char* name, int* argTypes, void**args);
int insertClientServerMessageToBuffer(char *messagePointer, char* name, int* argTypes, void**args);

#endif