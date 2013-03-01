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
};

bool operator == (const rpcFunctionKey &l, const rpcFunctionKey &r) 
{
	int c = strcmp(l,r);
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
	int c = strcmp(l,r);
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


typedef int (*skeleton)(int *, void **);
map<rpcFunctionKey, skeleton> registeredFunctions;

int rpcInit();
int rpcRegister(char *name, int *argTypes, skeleton f);


#endif