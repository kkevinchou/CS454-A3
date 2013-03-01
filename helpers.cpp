#include "helpers.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h> //getenv
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
using namespace std;

void error(string msg)
{
    cerr << msg << endl;
    exit(-1);
}

int setupSocketAndReturnDescriptor(char * serverAddressString, char * serverPortString)
{
    int socketFileDescriptor;
    int serverPort;

    struct sockaddr_in serverAddressStruct;
    struct hostent *server;

    serverPort = atoi(serverPortString);
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor < 0)
    {
        error("ERROR while opening socket");
    }

    server = gethostbyname(serverAddressString);

    if (server == NULL) {

       error("ERROR: No such host");

    }

    memset((char *) &serverAddressStruct, 0,sizeof(serverAddressStruct));

    serverAddressStruct.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serverAddressStruct.sin_addr.s_addr,
         server->h_length);
    serverAddressStruct.sin_port = htons(serverPort);

   // cout << "HOST: "<<serverAddressString << " PORT: " << serverPort<<endl;

    if (connect(socketFileDescriptor,(struct sockaddr *) &serverAddressStruct,sizeof(serverAddressStruct)) < 0)
        error("ERROR while connecting");

    return socketFileDescriptor;
}