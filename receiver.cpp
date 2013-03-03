#include "receiver.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <cstring>
#include "helpers.h"
#include "constants.h"
using namespace std;

static bool debug = false;

// Extract Arrays
unsigned short Receiver::getArrayLengthFromArgumentType(int argType)
{
    return argType & 65534;
}
int Receiver::getTypeFromArgumentType(int argType)
{
    return (argType >> 16) & 255;
}

char * Receiver::extractIntArray(char * head, int intArray[], unsigned int length) // make sure enough memory is in intArray
{
    for(int i = 0; i < length; i++)
    {
        int n;
        head = extractInt(head, n);
        intArray[i] = n;
    }
    return head;
}
char * Receiver::extractShortArray(char * head, short shortArray[], unsigned int length)
{
    for(int i = 0; i < length; i++)
    {
        short n;
        head = extractShort(head, n);
        shortArray[i] = n;
    }
    return head;
}
char * Receiver::extractCharArray(char * head, char charArray[], unsigned int length)
{
    for(int i = 0; i < length; i++)
    {
        char n;
        head = extractChar(head, n);
        charArray[i] = n;
    }
    return head;
}
char * Receiver::extractLongArray(char * head, long longArray[], unsigned int length)
{
    for(int i = 0; i < length; i++)
    {
        long n;
        head = extractLong(head, n);
        longArray[i] = n;
    }
    return head;
}
char * Receiver::extractDoubleArray(char * head, double doubleArray[], unsigned int length)
{
    for(int i = 0; i < length; i++)
    {
        double n;
        head = extractDouble(head, n);
        doubleArray[i] = n;
    }
    return head;
}
char * Receiver::extractFloatrray(char * head, float floatArray[], unsigned int length)
{
    for(int i = 0; i < length; i++)
    {
        float n;
        head = extractFloat(head, n);
        floatArray[i] = n;
    }
    return head;
}
// make sure enough memory is allocated in argTypes
char * Receiver::extractArgTypes(char * head, int argTypes[])
{
    int index = 0;
    while(true)
    {
        int i;
        head = extractInt(head, i);
        argTypes[index] = i;
        index++;
        if(i ==0) break;

    }
    return head;
}

// Extract Types
char * Receiver::extractLong(char * head, long &l)
{
    char buffer[4];
    for(int i = 0; i < 4; i++)
    {
        buffer[i] = *head;
        head++;
    }
    l = convertToLong(buffer);
    return head;
}
char * Receiver::extractFloat(char * head, float &f)
{
    char buffer[4];
    for(int i = 0; i < 4; i++)
    {
        buffer[i] = *head;
        head++;
    }
    f = convertToFloat(buffer);
    return head;
}
char * Receiver::extractChar(char * head, char &c)
{
    c = *head;
    head++;
    return head;
}
char * Receiver::extractDouble(char * head, double &d)
{
    char buffer[8];
    for(int i = 0; i < 8; i++)
    {
        buffer[i] = *head;
        head++;
    }
    d = convertToDouble(buffer);
    return head;
}

char * Receiver::extractUnsignedInt(char * head, unsigned int &i)
{
    char  buffer[4];
    for(int index = 0; index < 4; index++)
    {
        buffer[index] = *head;
        head++;
    }
    i = convertToUnsignedInt(buffer);
    return head;
}
char * Receiver::extractInt(char * head, int &i)
{
    char buffer[4];
    for(int index = 0; index < 4; index++)
    {
        buffer[index] = *head;
        head++;
    }
    i = convertToInt(buffer);
    return head;
}
char * Receiver::extractString(char * head, string &s)
{
    string r = "";
    while(*head != '\0')
    {
        //cout << "extracting "<< *head<<endl;
        r += *head;
        head++;
    }
    s = r;
    head++;
    return head;
}

char * Receiver::extractShort(char * head, short &i)
{
    char buffer[2];
    for(int index = 0; index < 2; index++)
    {
        buffer[index] = *head;
        head++;
    }
    i = convertToShort(buffer);
    return head;
}

//Constructor


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

unsigned int Receiver::returnArgTypesLength(char * head)
{
    // keeps incrementing pointer until we find a 0
    unsigned int counter = 0;
    while(true)
    {
        counter++;

        int type;
        head = extractInt(head, type);
        if(type == 0) break;
        
    }

    return counter;
}

// returns negative for error
int Receiver::receiveMessageSize(unsigned int &i)
{
    char buffer[4];

    if(receiveMessageGivenSize(4, buffer) == 0)
    {
        //cout << buffer[3]<<endl;
        i = convertToUnsignedInt(buffer);
        //cout << r << endl;
        return 0;
    }
    else return -1;
}

// Convert
short Receiver::convertToShort(char d[2])
{
    return (d[0] << 8) + d[1];
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


long Receiver::convertToLong(char d[4])
{
return (d[0] << 24) + (d[1] << 16) + (d[2] << 8) + d[3];
}
float Receiver::convertToFloat(char d[4])
{
return (d[0] << 24) + (d[1] << 16) + (d[2] << 8) + d[3];
}
double Receiver::convertToDouble(char d[8])
{
    return *reinterpret_cast<double*>(d);
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
        if (debug) cerr << "Receiving..."<<sizeLeft<<endl;
        if(n == 0)
        {
            cerr << "Connection closed"<<endl;
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

    if (debug)
    {
        if (debug) cerr << "Received: ";
        for(unsigned int i = 0; i < messageSize; i++)
        {
            cerr << (int)ret[i] << " ";
        }
        cout << endl;
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
    }
  

    return "";
    

}




