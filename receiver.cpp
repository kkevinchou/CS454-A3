#include "receiver.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <cstring>
#include "helpers.h"
#include "constants.h"
using namespace std;

bool debug = true;

Receiver::Receiver(int socketFileDescriptor)
{
	_sfd = socketFileDescriptor;
}
MessageType Receiver::receiveMessageType()
{
    char buffer[4];

    if(receiveMessageGivenSize(4, buffer) == 0)
    {
        //cout << buffer[3]<<endl;
        unsigned int r = convertToInt(buffer);
        //cout << r << endl;

        return getMessageTypeFromInt((int)r);
    }
    else return ERROR;
}



// returns negative for error
int Receiver::receiveMessageSize()
{
    char buffer[4];

    if(receiveMessageGivenSize(4, buffer) == 0)
    {
        //cout << buffer[3]<<endl;
        unsigned int r = convertToUnsignedInt(buffer);
        //cout << r << endl;
        return r;
    }
    else return -1;
}
int Receiver::convertToInt(char d[4])
{
    return (d[0] << 24) + (d[1] << 16) + (d[2] << 8) + d[3];
}
// takes in a length 4 char *
unsigned int Receiver::convertToUnsignedInt(char d[4])
{
    return (d[0] << 24) + (d[1] << 16) + (d[2] << 8) + d[3];
}

// returns a message char * of length MessageSize (including null termination char if one exists)
int Receiver::receiveMessageGivenSize(unsigned int messageSize, char ret[])
{
    if (debug) cerr << "Receiving size " << messageSize <<endl;
    if(messageSize == 0) return 0;

    int n;

    char * bufferPointer = ret;
    unsigned int sizeLeft = messageSize;
    while(true)
    {
        memset(bufferPointer,0,sizeLeft);
        n = recv(_sfd,bufferPointer,sizeLeft, 0);
        if(n == 0)
        {
            return -1; //connection closed!
        }
        else if (n < 0)
        {
            cerr << "ERROR reading from socket"<<endl;
            return -2;
        }
         bufferPointer += n;
         sizeLeft -= n;
         if(sizeLeft <= 0) break;

    }

    if (debug) cerr << "Received: ";
    for(int i = 0; i < messageSize; i++)
    {
        if (debug) cerr << (int)ret[i] << " ";
    }
    cout << endl;
    return 0;
}

// deprecated
string Receiver::receiveMessageAsString()
{
    unsigned int messageSize = receiveMessageSize();
    char msg[messageSize];
   // cout << "A"<<endl;
    if(receiveMessageGivenSize(messageSize, msg) == 0)
    {
       // cout << "B"<<endl;
        // if we're expecting a string, messageSize should include the null char
        // but when we're creating a string from it, don't consider the last null char
        string ret(msg, msg+messageSize-1);
       // cout << "C"<<endl;
        return ret;
    }
    else
    {
        return "";
    }

}