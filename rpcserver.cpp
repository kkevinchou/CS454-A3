#include "rpcserver.h"
#include "helpers.h"
#include "constants.h"
#include <stdlib.h> //getenv
#include <string>
#include <iostream>

using namespace std;

int localSocketFd;
int binderSocketFd;



map<rpcFunctionKey, skeleton> registeredFunctions;

bool operator == (const rpcFunctionKey &l, const rpcFunctionKey &r) 
{
	int c = strcmp(l.name,r.name);
	if(c == 0)
	{
		int i = 0;
		int lArgs = l.argTypes[i];
		int rArgs = r.argTypes[i];
		// structs with less args are smaller
		// structs with the same args are the same
		// structs with the same number args, 
		// but different arg values, 
		// the one with the smaller value is smaller
		while(lArgs!= 0 && rArgs != 0)
		{
			if(lArgs != rArgs)
			{
				return false;
			}
			i++;
			lArgs = l.argTypes[i];
			rArgs = r.argTypes[i];
		}	

		if(rArgs == 0 && lArgs == 0)
		{
			// have same number of args with same type
			return true;
		}
		else
		{
			// have different number of args
			return false;
		}
	}
	else
	{
	 	return false;
	}
}

bool operator < (const rpcFunctionKey &l, const rpcFunctionKey &r) 
{ 
	int c = strcmp(l.name,r.name);
	if(c == 0)
	{
		int i = 0;
		int lArgs = l.argTypes[i];
		int rArgs = r.argTypes[i];
		// structs with less args are smaller
		// structs with the same args are the same
		// structs with the same number args, 
		// but different arg values, 
		// the one with the smaller value is smaller
		while(lArgs!= 0 && rArgs != 0)
		{
			if(lArgs != rArgs)
			{
				return lArgs < rArgs;
			}
			i++;
			lArgs = l.argTypes[i];
			rArgs = r.argTypes[i];
		}	

		if(rArgs == 0) // || lArgs == 0
		{
			// l is longer, so l > r
			// or same length with same parameters, so l == r, which means l < r is false
			return false;
		}
		else
		{
			// r is longer, so l < r
			return true;
		}
	}
	else
	{
		// c > 0 if l > r
		// c < 0 if l < r
		return (c < 0);
	}
}

int rpcInit() {
    char * binderAddressString = getenv ("BINDER_ADDRESS");
    char * binderPortString = getenv("BINDER_PORT");

    if(binderAddressString == NULL) error("ERROR: BINDER_ADDRESS environment variable not set.");
    if(binderPortString == NULL) error("ERROR: BINDER_PORT environment variable not set.");

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
	struct rpcFunctionKey k(name, argTypes);

	if(registeredFunctions[k] != NULL) return 1;
	registeredFunctions[k] = f;

	return 0;
}