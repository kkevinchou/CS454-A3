#ifndef HELPERS_H
#define HELPERS_H

#include <string>

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

void listenOnSocket(int localSocketFd);
int acceptConnection(int localSocketFd);

#endif