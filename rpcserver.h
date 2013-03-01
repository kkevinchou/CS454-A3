#ifndef RPCSERVER_H
#define RPCSERVER_H

typedef int (*skeleton)(int *, void **);

void rpcInit();
int rpcRegister(char *name, int *argTypes, skeleton f);


#endif