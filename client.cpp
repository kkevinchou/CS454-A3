
#include <iostream>
#include <stdio.h>
#include <stdlib.h> //getenv
#include <unistd.h>


#include "helpers.h"
#include "sender.h"
#include "receiver.h"
#include "rpc.h"
#include "constants.h"
using namespace std;


int main()
{
	  int argTypes0[4];
  void **args0;
    int a0 = 5;
  int b0 = 10;
  int count0 = 3;
  int return0 = 0;

	 argTypes0[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  argTypes0[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
  argTypes0[2] = (1 << ARG_INPUT) | (ARG_INT << 16);
  argTypes0[3] = 0;

  args0 = (void **)malloc(3 * sizeof(void *));
  args0[0] = (void *)&return0;
  args0[1] = (void *)&a0;
  args0[2] = (void *)&b0;

  rpcCall("f0", argTypes0, args0);
/*	char * serverAddressString = getenv ("SERVER_ADDRESS");
    char * serverPortString = getenv("SERVER_PORT");

    if(serverAddressString == NULL) error("ERROR: SERVER_ADDRESS environment variable not set.");
    if(serverPortString == NULL) error("ERROR: SERVER_PORT environment variable not set.");

	int socketFileDescriptor = setupSocketAndReturnDescriptor(serverAddressString, serverPortString);

	Sender s(socketFileDescriptor);
	Receiver r(socketFileDescriptor);
	s.sendMessage("hi kevin, this is a test");
s.sendMessage("byebye");
	string t = r.receiveMessageAsString();
	cout << "received: " << t << endl;
	t = r.receiveMessageAsString();
	cout << "received: " << t << endl;
	close(socketFileDescriptor);*/


}