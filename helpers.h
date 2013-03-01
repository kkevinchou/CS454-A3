#ifndef HELPERS_H
#define HELPERS_H

#include <string>

using namespace std;
void error(string msg);
int setupSocketAndReturnDescriptor(char * serverAddressString, char * serverPortString);

#endif