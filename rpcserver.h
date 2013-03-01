#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <map>
#include <string>
#include <string.h>
#include <stdio.h>


using namespace std;

struct rpcFunctionKey
{ 
	char *name;
	int *argTypes;
	rpcFunctionKey(char *n, int *a) : name(n), argTypes(a){}
};




typedef int (*skeleton)(int *, void **);


int rpcInit();
int rpcRegister(char *name, int *argTypes, skeleton f);


#endif