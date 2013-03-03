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

static map<rpcFunctionKey, skeleton> registeredFunctions;

int rpcInit() {
    char * binderAddressString = getenv ("BINDER_ADDRESS");
    char * binderPortString = getenv("BINDER_PORT");

    if(binderAddressString == NULL) error("ERROR: BINDER_ADDRESS environment variable not set.");
    if(binderPortString == NULL) error("ERROR: BINDER_PORT environment variable not set.");

    cerr << "connecting to : " << binderAddressString << ":" << binderPortString << endl;
    binderSocketFd = setupSocketAndReturnDescriptor(binderAddressString, binderPortString);

    if (binderSocketFd < 0) {
        return binderSocketFd;
    }

    localSocketFd = createSocket();

    if (localSocketFd < 0) {
        return localSocketFd;
    }

    return 0;
}

// returns 0 on sucess
// warnings:
// 1 - function already added
// errors:
// -1 - binder could not be found
int rpcRegister(char *name, int *argTypes, skeleton f) {
	// calls the binder, informing it that a server procedure
	// with the indicated name and list of argument types is available at this server.

	// The result returned is 0 for a successful registration, positive for a warning
	// (e.g., this is the same as some previously registered procedure), or negative
	// for failure (e.g., could not locate binder).

	// The function also makes an entry in a local database,
	// associating the server skeleton with the name and list of argument types.
	struct rpcFunctionKey k(string(name), argTypes);

	if(registeredFunctions[k] != NULL) return 1;
	registeredFunctions[k] = f;

	string hostname = getHostname();
	short port = getPort(localSocketFd);
	string funcName = string(name);

	Sender s(binderSocketFd);

	int argTypesLength = 0;

	while (argTypes[argTypesLength++]);

	cerr << "hostname : " << hostname.size() << endl;
	cerr << "funcName : " << funcName.size() << endl;
	cerr << "argTypesLength : " << argTypesLength << endl;

	s.sendRegisterMessage(hostname, port, funcName, argTypesLength, argTypes);

	cout << "Successfully registered: "<< name << endl;

	return 0;
}

int rpcExecute()
{
	return 0;
}
