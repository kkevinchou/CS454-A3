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

int rpcCall(char* name, int* argTypes, void** args) {
    cerr << "RPC CALL" << endl;
    return sendLocRequest(string(name), argTypes);
}