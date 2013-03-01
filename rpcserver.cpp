#include "rpcserver.h"
#include "helpers.h"
#include "constants.h"

int localSocketFd;
int binderSocketFd;

int rpcInit() {
    char * binderAddressString = getenv ("BINDER_ADDRESS");
    char * binderPortString = getenv("BINDER_PORT");

    if(binderAddressString == NULL) error("ERROR: SERVER_ADDRESS environment variable not set.");
    if(binderPortString == NULL) error("ERROR: SERVER_PORT environment variable not set.");

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

int rpcRegister(char *name, int *argTypes, skeleton f) {
	struct rpcFunctionKey k(name, argTypes);
	registeredFunctions[k] = f;
}