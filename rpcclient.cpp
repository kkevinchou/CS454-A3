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

static bool debug = true;
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
    s.sendLocRequestMessage(name, argTypes);

    return 0;
}


int sendExecuteRequest(char* name, int* argTypes, void**args)
{
    RWBuffer b;
    Sender s(binderSocketFd);

    unsigned int messageSize = getClientServerMessageLength(name, argTypes, args);



    if(debug)
    {
        cout << endl;
        cout << "Name: " << name<<endl;
    }
    // create buffer for full message
    char message[messageSize];

   // char * messagePointer = message;
  //  messagePointer = b.insertUnsignedIntToBuffer(messageSize, messagePointer);
   // messagePointer = b.insertIntToBuffer(static_cast<int>(EXECUTE), messagePointer);

   insertClientServerMessageToBuffer(message, name, argTypes, args);

    if(debug)
    {
        cout << "Sending an EXECUTE message of size "<<messageSize << ": "<<endl;
        for(int i = 0; i < messageSize; i++)
        {
            cout << (int)message[i] << " ";

        }
        cout << endl;
    }
    int r = s.sendMessage(messageSize, EXECUTE, message);

    return r;
}

int processLocResponse(string &serverID, unsigned short &port) {
    Receiver r(binderSocketFd);

    unsigned int messageSize;

    r.receiveMessageSize(messageSize);
    MessageType msgType = r.receiveMessageType();

    char buffer[messageSize];
    char *bufferPointer = buffer;
    r.receiveMessageGivenSize(messageSize, buffer);

    RWBuffer b;

    if (msgType == LOC_SUCCESS) {
        bufferPointer = b.extractString(bufferPointer, serverID);
        bufferPointer = b.extractUnsignedShort(bufferPointer, port);
        return 0;
    } else if (msgType == LOC_FAILURE) {
        cerr << "FAILURE" << endl;
        // TODO: We don't do anything with the reason code for now...
        return -1;
    } else {
        cerr << "UNEXPECTED MSGTYPE IN processLocResponse()" << endl;
        return -1;
    }
}

int rpcCall(char* name, int* argTypes, void** args) {
    // cerr << "RPC CALL" << endl;
    sendLocRequest(string(name), argTypes);

    string serverID;
    unsigned short port = 13;

    processLocResponse(serverID, port);

    cerr << "SERVER : " << serverID << endl;
    cerr << "PORT : " << port << endl;
    // char * binderAddressString = getenv ("BINDER_ADDRESS");
    // char * binderPortString = getenv("BINDER_PORT");
    // binderSocketFd = setupSocketAndReturnDescriptor(binderAddressString, binderPortString);

    // cout << "SENDING EXECUTE "<<sendExecuteRequest(name, argTypes, args)<<endl;
    return 0;
}