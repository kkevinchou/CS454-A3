#include "receiver.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <cstring>
#include "helpers.h"
using namespace std;

Receiver::Receiver(int socketFileDescriptor)
{
	_sfd = socketFileDescriptor;	
}

// returns negative for error
int Receiver::receiveMessageSize()
{
    char buffer[4];

    if(receiveMessageGivenSize(4, buffer) == 0)
    {
         return convertToUnsignedInt(buffer);
    }
    else return -1;
}

// takes in a length 4 char *
unsigned int Receiver::convertToUnsignedInt(char d[4])
{
    return (d[0] << 24) + (d[1] << 16) + (d[2] << 8) + d[3];
}

// returns a message char * of length MessageSize (including null termination char if one exists)
int Receiver::receiveMessageGivenSize(unsigned int messageSize, char ret[])
{
   // string msg = "";
    if(messageSize == 0) return 0;

    int n;

    char * bufferPointer = ret;
    while(true)
    {
      // receive the message
        memset(bufferPointer,0,messageSize);
        n = recv(_sfd,bufferPointer,messageSize, 0);

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

         messageSize -= n;
         if(messageSize <= 0) break;

    }
    return 0;
}

// deprecated
string Receiver::receiveMessageAsString()
{
    unsigned int messageSize = receiveMessageSize();
    char msg[messageSize];
    if(receiveMessageGivenSize(messageSize, msg) == 0)
    {
        string ret(msg, msg+messageSize);
        return ret;
    }
    else
    {
        return "";
    }

}