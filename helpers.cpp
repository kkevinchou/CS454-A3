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
#include "constants.h"
#include "rpc.h"
using namespace std;

unsigned int sizeOfType(int type)
{
    switch(type)
    {
        case ARG_CHAR:
            return 1;
        break;
        case ARG_SHORT:
            return 2;
        break;
        case ARG_INT:
             return 4;
        break;
        case ARG_LONG:
            return 4;
        break;
        case ARG_DOUBLE:
            return 8;
        break;
        case ARG_FLOAT:
            return 4;
        break;
        default:
            return 0;
        break;
    }
    return 0;
}
MessageType getMessageTypeFromInt(int i)
{
    MessageType ret = ERROR;
    switch(i)
    {
        case 1:
            ret = REGISTER;
        break;
        case 2:
            ret = LOC_REQUEST;
        break;
        case 3:
            ret = LOC_SUCCESS;
        break;
        case 4:
            ret = LOC_FAILURE;
        break;
        case 5:
            ret = EXECUTE;
        break;
        case 6:
            ret = EXECUTE_SUCCESS;
        break;
        case 7:
            ret = EXECUTE_FAILURE;
        break;
        case 8:
            ret = TERMINATE;
        break;
        default:
        break;
    }
    return ret;
}
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
        return -2;
    }

    server = gethostbyname(serverAddressString);

    if (server == NULL) {

       return -3;

    }

    memset((char *) &serverAddressStruct, 0,sizeof(serverAddressStruct));

    serverAddressStruct.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serverAddressStruct.sin_addr.s_addr,
         server->h_length);
    serverAddressStruct.sin_port = htons(serverPort);

   // cout << "HOST: "<<serverAddressString << " PORT: " << serverPort<<endl;

    if (connect(socketFileDescriptor,(struct sockaddr *) &serverAddressStruct,sizeof(serverAddressStruct)) < 0)
    {
        return -4;
    }

    return socketFileDescriptor;
}

int createSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

void listenOnSocket(int localSocketFd) {
    struct sockaddr_in binderAddress;
    memset((struct sockaddr_in *)&binderAddress, 0, sizeof(binderAddress));

    binderAddress.sin_family = AF_INET;
    binderAddress.sin_addr.s_addr = INADDR_ANY;
    binderAddress.sin_port = 0;

    if (bind(localSocketFd, (struct sockaddr *) &binderAddress, sizeof(binderAddress)) < 0)
          error("ERROR: Failed to bind local socket");

    listen(localSocketFd, 5);
}

int acceptConnection(int localSocketFd) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int newSocketFd = accept(localSocketFd, (struct sockaddr *) &clientAddress, &clientAddressSize);

    if (newSocketFd < 0)
        error("ERROR: Failed to accept client connection");

    return newSocketFd;
}

short getPort(int localSocketFd) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(localSocketFd, (struct sockaddr *)&sin, &len);
    return ntohs(sin.sin_port);
}

string getHostname() {
    char localHostName[256];
    gethostname(localHostName, 256);

    return string(localHostName);
}