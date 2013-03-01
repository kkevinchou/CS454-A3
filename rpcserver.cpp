#include "rpcserver.hpp"
#include "helpers.h"
void rpcInit() {
    char * binderAddressString = getenv ("BINDER_ADDRESS");
    char * binderPortString = getenv("BINDER_PORT");

    if(binderAddressString == NULL) error("ERROR: SERVER_ADDRESS environment variable not set.");
    if(binderPortString == NULL) error("ERROR: SERVER_PORT environment variable not set.");

    int binderSocketFd = setupSocketAndReturnDescriptor(binderAddressString, binderPortString);
    int localSocketFd = createSocket();
}

int rpcRegister(char *name, int *argTypes, skeleton f) {

}