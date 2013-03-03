#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <map>
#include <string>
#include <string.h>
#include <stdio.h>

using namespace std;

typedef int (*skeleton)(int *, void **);
int rpcInit();
int rpcRegister(char *name, int *argTypes, skeleton f);


#endif