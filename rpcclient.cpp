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
#include "rwbuffer.h"
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
    RWBuffer b;
    Sender s(binderSocketFd);
    unsigned int argTypesLength = 0;
    unsigned int messageSize = 0;

    // calculate length of arguments
    int * argTypesP = argTypes;
    while(*argTypesP != 0)
    {
        int argType = *argTypesP;

        unsigned short length = b.getArrayLengthFromArgumentType(argType);
        if(length == 0) length = 1; // if it's a scalar, it still takes up one room

        int type = b.getTypeFromArgumentType(argType);
        unsigned int size = sizeOfType(type);
        messageSize += length*size;
//cout << "type "<<type << " size "<< size << endl;
        argTypesP++;
        argTypesLength++;
    }
    argTypesLength++; //accout for the 0

    // calculate length of argTypes
    messageSize += 4*argTypesLength;



    // calculate name size
    char * nameP = name;
    while(*nameP != '\0')
    {
        messageSize++;
        nameP++;
    }
    messageSize++; //accountfor null termination char

    // create buffer for full message
    char message[messageSize + 8];

    char * messagePointer = message;
    messagePointer = b.insertUnsignedIntToBuffer(messageSize, messagePointer);
    messagePointer = b.insertIntToBuffer(static_cast<int>(EXECUTE), messagePointer);
    messagePointer = b.insertCStringToBuffer(name, messagePointer);

    messagePointer = b.insertIntArrayToBuffer(argTypes, argTypesLength, messagePointer);
    for(int i = 0; i < argTypesLength-1; i++)
    {
         int argType = argTypes[i];

        unsigned short length = b.getArrayLengthFromArgumentType(argType);
        if(length == 0) length= 1;
        int type = b.getTypeFromArgumentType(argType);/*
        char *addShortToBuffer(short s, char *bufferP);
        char *addIntToBuffer(int i, char *bufferP);
        char *addStringToBuffer(string s, char *bufferP);
        char *addIntBufferToBuffer(int intBuf[], int numInts, char *bufferP);
        char *addUnsignedIntToBuffer(unsigned int u, char *bufferP);*/

        void * arg = args[i];
        cout << "type "<<type<<endl;
        switch(type)
        {
            case ARG_CHAR:
            //messagePointer = addCharToBuffer()

            break;
            case ARG_SHORT:

            break;
            case ARG_INT:
            {
                int * ints = (int *)arg;
                for(int j = 0; j < length; j++)
                {
                    cout << "HERE "<<ints[j]<<endl;
                    messagePointer = b.insertIntToBuffer(ints[j], messagePointer);
                }
            }
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

    cout << "Message "<<messageSize<<": "<<endl;
    for(int i = 0; i < messageSize+8; i++)
    {
        cout << (int)message[i] << " ";

    }
    cout << endl;
    s.sendArray(messageSize + 8, message);
}
int rpcCall(char* name, int* argTypes, void** args) {
    // cerr << "RPC CALL" << endl;
    sendLocRequest(string(name), argTypes);
    // char * binderAddressString = getenv ("BINDER_ADDRESS");
    // char * binderPortString = getenv("BINDER_PORT");
    // binderSocketFd = setupSocketAndReturnDescriptor(binderAddressString, binderPortString);

    // cout << "SENDING EXECUTE "<<sendExecuteRequest(name, argTypes, args)<<endl;
    return 0;
}