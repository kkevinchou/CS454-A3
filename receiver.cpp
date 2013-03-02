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
        //cout << buffer[3]<<endl;
        unsigned int r = convertToUnsignedInt(buffer);
        //cout << r << endl;
         return r;
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
   // cout << "Receiving size "<<messageSize<<endl;
   // string msg = "";
    if(messageSize == 0) return 0;

    int n;

    char * bufferPointer = ret;
   // cout << "a "<<endl;
    unsigned int sizeLeft = messageSize;
    while(true)
    {
      // receive the message
        memset(bufferPointer,0,sizeLeft);
       // cout << "b "<<endl;
        n = recv(_sfd,bufferPointer,sizeLeft, 0);
//cout << "c "<<endl;
        if(n == 0)
        {
            return -1; //connection closed!
        }
        else if (n < 0)
         {
             cerr << "ERROR reading from socket"<<endl;
             return -2;
         }
        // cout << n << endl;
         bufferPointer += n;
         //cout << "d"<<endl;
         sizeLeft -= n;
         if(sizeLeft <= 0) break;

    }

    /*cout << "Received: ";
    for(int i = 0; i < messageSize; i++)
    {
        cout << ret[i] << " ";
    }
    cout << endl;*/
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