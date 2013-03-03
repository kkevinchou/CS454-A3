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

    bool debug = true;

    if(debug)
    {
        cout << endl;
        cout << "Name: " << name<<endl;
    }
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

        switch(type)
        {
            case ARG_CHAR:
            {
                char * chars = (char *)arg;
                if(debug) cout << " "<< i<<": Char ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << chars[j]<< " ";
                    messagePointer = b.insertCharToBuffer(chars[j], messagePointer);
                }

                if(debug) cout << endl;
            }

            break;
            case ARG_SHORT:
            {
                short * shorts = (short *)arg;
                if(debug) cout << " "<< i<<": Short ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << shorts[j] << " ";
                    messagePointer = b.insertShortToBuffer(shorts[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            break;
            case ARG_INT:
            {
                int * ints = (int *)arg;
                if(debug) cout << " "<< i<<": Int ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << ints[j] << " ";
                    messagePointer = b.insertIntToBuffer(ints[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            case ARG_LONG:
            {
                long * longs = (long *)arg;
                if(debug) cout << " "<< i<<": Long ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << longs[j] << " ";
                    messagePointer = b.insertLongToBuffer(longs[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            case ARG_DOUBLE:
            {
                double * doubles = (double *)arg;
                if(debug) cout << " "<< i<<": Double ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << doubles[j] << " ";
                    messagePointer = b.insertDoubleToBuffer(doubles[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            case ARG_FLOAT:
            {
                float * floats = (float *)arg;
                if(debug) cout << " "<< i<<": Float ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << floats[j] << " ";
                    messagePointer = b.insertFloatToBuffer(floats[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            break;
            default:
                cerr << "WARNING: argument of unknown type."<<endl;
            break;
        }
    }
    if(debug)
    {
        cout << "Sending an EXECUTE message of size "<<messageSize << ": "<<endl;
        for(int i = 0; i < messageSize+8; i++)
        {
            cout << (int)message[i] << " ";

        }
        cout << endl;
    }

    s.sendArray(messageSize + 8, message);
}
int rpcCall(char* name, int* argTypes, void** args) {
    cerr << "RPC CALL" << endl;
        char * binderAddressString = getenv ("BINDER_ADDRESS");
    char * binderPortString = getenv("BINDER_PORT");
    binderSocketFd = setupSocketAndReturnDescriptor(binderAddressString, binderPortString);

    sendExecuteRequest(name, argTypes, args);
    //return sendLocRequest(string(name), argTypes);
    return 0;
}