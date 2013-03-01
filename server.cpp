#include <iostream>

#include "rpcserver.h"
#include "constants.h"

using namespace std;

int sum (int a, int b)
{
	return a+b;
}
// return -1 for wrong argument
int sumSkeleton(int * argTypes, void **args)
{
	int index = 0;
	int argType = argTypes[index];
	int returnValueIndex = -1;

	int aIndex;
	int bIndex;
	int aSet = false;
	int bSet = false;

	while(argType !=0)
	{
		int isOutput = (argType >> ARG_OUTPUT) & 1;
		int type = (argType >> 16) & 256; // 8 bits
		int arrayLength = argType & 65536; // 16 bits

		if(isOutput)
		{
			if(returnValueIndex >= 0) return -1; // more than one return arguments, sum only has one
			if(type != ARG_INT) return -1; // return value not int
			if(arrayLength > 0) return -1; // not a scalar
			returnValueIndex = index;
		}
		else
		{

			int isInput = (argType >> ARG_INPUT) & 1;
			if(isInput)
			{
				if(type != ARG_INT || arrayLength > 0) return -1; // wrong argument types

				if(!aSet)
				{
					aIndex = index;
					aSet = true;
				}
				else if(!bSet)
				{
					bIndex = index;
					bSet = true;
				}
				else return -1;//to many arguments

			}

		}


		index ++;
		argType = argTypes[index];
	}

	int a = *((int*)(args[aIndex]));
	int b = *((int*)(args[bIndex]));
	int s = sum(a,b);
	// do server function call
	*((int*)(args[returnValueIndex])) = s;
	return 0; // success!
}

int main()
{
	rpcInit();

	// register sum
	int argTypes[3];
	argTypes[0] = (1 << ARG_INPUT) | (ARG_INT << 16);
	argTypes[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
	argTypes[2] = (1 << ARG_OUTPUT) | (ARG_INT << 16);

	char *name = new char[4];
	strcpy( name, "sum" );

	int r = rpcRegister(name, argTypes, sumSkeleton);


}