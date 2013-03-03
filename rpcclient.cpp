#include "rpc.h"
#include "helpers.h"
#include <stdlib.h> //getenv
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <map>
#include "sender.h"
#include "constants.h"
#include "receiver.h"
using namespace std;

static int localSocketFd;
static int binderSocketFd;

int sendLocRequest(string name, int argTypes[]) {
    char * binderAddressString = getenv ("BINDER_ADDRESS");
    char * binderPortString = getenv("BINDER_PORT");

    if(binderAddressString == NULL) error("ERROR: BINDER_ADDRESS environment variable not set.");
    if(binderPortString == NULL) error("ERROR: BINDER_PORT environment variable not set.");

    cerr << "connecting to : " << binderAddressString << ":" << binderPortString << endl;
    binderSocketFd = setupSocketAndReturnDescriptor(binderAddressString, binderPortString);

    if (binderSocketFd < 0) {
        return binderSocketFd;
    }

    Sender s(binderSocketFd);

    return s.sendLocRequestMessage(name, argTypes);
}


int sendExecuteRequest(char* name, int* argTypes, void**args)
{
    Receiver r(0);
    unsigned int argTypesLength = 1;
    unsigned int messageSize = 0;
    int * argTypesP = argTypes;
    while(*argTypesP != 0)
    {
        int argType = *argTypesP;

        unsigned short length = r.getArrayLengthFromArgumentType(argType);
        if(length == 0) length = 1; // if it's a scalar, it still takes up one room

        int type = getTypeFromArgumentType(argType);
        unsignedint sizeOfType = sizeOfType(type);
        messageSize += length*sizeOfType;
{
        argTypesP++;
        argTypesLength++;
    }

    messageSize += 4*argTypesLength;

    char message[messageSize + 8];

    char * messagePointer = message;
    messagePointer = addUnsignedIntToBuffer(messageSize, messagePointer);
    messagePointer = addIntToBuffer(static_cast<int>(EXECUTE), messagePointer);
    for(int i = 0; i < argTypesLength-1; i++)
    {
         int argType = *argTypesP;
        unsigned short length = r.getArrayLengthFromArgumentType(argType);
        if(length == 0) length= 1;
        int type = getTypeFromArgumentType(argType);/*
        char *addShortToBuffer(short s, char *bufferP);
        char *addIntToBuffer(int i, char *bufferP);
        char *addStringToBuffer(string s, char *bufferP);
        char *addIntBufferToBuffer(int intBuf[], int numInts, char *bufferP);
        char *addUnsignedIntToBuffer(unsigned int u, char *bufferP);*/
        switch(type)
        {
            case ARG_CHAR:
            messagePointer = addCharToBuffer()

            break;
            case ARG_SHORT:

            break;
            case ARG_INT:

            break;
            case ARG_LONG:

            break;
            case ARG_DOUBLE:

            break;
            case ARG_FLOAT:

            break;
            default:

            break;
        }
    }


    sendArray(messageSize + 8, message);
}
int rpcCall(char* name, int* argTypes, void** args) {
    cerr << "RPC CALL" << endl;
    return sendLocRequest(string(name), argTypes);
}