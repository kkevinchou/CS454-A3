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

unsigned int Receiver::receiveMessageSize()
{
    int n;
    unsigned char sizeBuffer[4];
    unsigned int sizeSize = 4;
    unsigned char * sizeBufferP = sizeBuffer;

    while(true)
    {
        memset(sizeBuffer,0,4);
        n = recv(_sfd,sizeBufferP,sizeSize, 0);
        if(n==0)
        {
            return NULL; //connection closed!
        }
        else if (n < 0)
        {
             cerr << "ERROR reading from socket"<<endl;
             return NULL;
         }

         sizeBufferP += n;
         sizeSize -= n;

         if(sizeSize == 0) break;
        //receive 4 bytes first
    }

    unsigned int messageSize = (sizeBuffer[0] << 24) + (sizeBuffer[1] << 16) + (sizeBuffer[2] << 8) + sizeBuffer[3];
    return messageSize;
}
string Receiver::receiveMessageGivenSize(unsigned int messageSize)
{
    string msg = "";
    int n;
    char buffer[messageSize];
    while(true)
    {
      // receive the message
        memset(buffer,0,messageSize);
        n = recv(_sfd,buffer,messageSize, 0);

        if(n == 0)
        {
            return NULL; //connection closed!
        }
        else if (n < 0)
         {
             cerr << "ERROR reading from socket"<<endl;
             return NULL;
         }

         string line(buffer);
         msg += line;

         messageSize -= n;
         if(messageSize <= 0) break;

    }
    return msg;
}
string Receiver::receiveMessage()
{
    unsigned int messageSize = receiveMessageSize();
    return receiveMessageGivenSize(messageSize);
}