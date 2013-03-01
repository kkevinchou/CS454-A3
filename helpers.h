#ifndef HELPERS_H
#define HELPERS_H

#include <string>

using namespace std;

void error(string msg);
int setupSocketAndReturnDescriptor(char * serverAddressString, char * serverPortString);

int createSocket();

// Server Helpers

void listenOnSocket(int localSocketFd);
int acceptConnection(int localSocketFd);

#endif