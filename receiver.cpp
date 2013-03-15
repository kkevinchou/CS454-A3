#include "receiver.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <cstring>
#include "helpers.h"
#include "constants.h"
#include "rwbuffer.h"
using namespace std;

static bool debug = false;

//Constructor
int Receiver::getSocketFD()
{
    return _sfd;
}

Receiver::Receiver(int socketFileDescriptor)
{
	_sfd = socketFileDescriptor;
}

int Receiver::receiveMessageType(MessageType &m)
{
    char buffer[4];
    int n = receiveMessageGivenSize(4, buffer);
    if(n == 0)
    {
        RWBuffer b;
        int r;
        b.extractInt(buffer, r);

        m = getMessageTypeFromInt(r);
        return 0;
    }
    else return n;
}

// returns negative for error
int Receiver::receiveMessageSize(unsigned int &i)
{
    char buffer[4];

    if(receiveMessageGivenSize(4, buffer) == 0)
    {
        RWBuffer b;
        b.extractUnsignedInt(buffer, i);
        return 0;
    }
    else return -1;
}

// returns a message char * of length MessageSize (including null termination char if one exists)
int Receiver::receiveMessageGivenSize(unsigned int messageSize, char ret[])
{
    if(messageSize == 0) return 0;

    int n;

    char * bufferPointer = ret;
    unsigned int sizeLeft = messageSize;
    while(sizeLeft > 0)
    {
        memset(bufferPointer,0,sizeLeft);
        n = recv(_sfd,bufferPointer,sizeLeft, 0);
        if(n == 0)
        {
            return SOCKET_CONNECTION_FALIURE; //connection closed!
        }
        else if (n < 0)
        {
            return SOCKET_RECEIVE_FAILURE;
        }
         bufferPointer += n;
         sizeLeft -= n;

    }

    return 0;
}

// deprecated
string Receiver::receiveMessageAsString()
{
    unsigned int messageSize;
    if(receiveMessageSize(messageSize) == 0)
    {
        char msg[messageSize];
        if(receiveMessageGivenSize(messageSize, msg) == 0)
        {
            string ret(msg, msg+messageSize-1);
            return ret;
        }
    }
    return "";
}