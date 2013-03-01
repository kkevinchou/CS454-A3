
#include <iostream>
#include <stdio.h>
#include <stdlib.h> //getenv
#include <unistd.h>


#include "helpers.h"
#include "sender.h"
using namespace std;


int main()
{
	
	char * serverAddressString = getenv ("SERVER_ADDRESS");
    char * serverPortString = getenv("SERVER_PORT");

    if(serverAddressString == NULL) error("ERROR: SERVER_ADDRESS environment variable not set.");
    if(serverPortString == NULL) error("ERROR: SERVER_PORT environment variable not set.");

	int socketFileDescriptor = setupSocketAndReturnDescriptor(serverAddressString, serverPortString);

	Sender s(socketFileDescriptor);
	s.sendMessage("Hi kevin");
	close(socketFileDescriptor);
}