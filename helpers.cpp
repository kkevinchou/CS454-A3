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
#include "rwbuffer.h"
using namespace std;
static bool debug = false;
unsigned int sizeOfType(int type)
{
    switch(type)
    {
        case ARG_CHAR:
            return sizeof(char);
        break;
        case ARG_SHORT:
            return sizeof(short);
        break;
        case ARG_INT:
             return sizeof(int);
        break;
        case ARG_LONG:
            return sizeof(long);
        break;
        case ARG_DOUBLE:
            return sizeof(double);
        break;
        case ARG_FLOAT:
            return sizeof(float);
        break;
        default:
            return 0;
        break;
    }
    return 0;
}
MessageType getMessageTypeFromInt(int i)
{
    return static_cast<MessageType>(i);
    return (MessageType)i;
    // MessageType ret = ERROR;
    // switch(i)
    // {
    //     case 1:
    //         ret = REGISTER;
    //     break;
    //     case 2:
    //         ret = LOC_REQUEST;
    //     break;
    //     case 3:
    //         ret = LOC_SUCCESS;
    //     break;
    //     case 4:
    //         ret = LOC_FAILURE;
    //     break;
    //     case 5:
    //         ret = EXECUTE;
    //     break;
    //     case 6:
    //         ret = EXECUTE_SUCCESS;
    //     break;
    //     case 7:
    //         ret = EXECUTE_FAILURE;
    //     break;
    //     case 8:
    //         ret = TERMINATE;
    //     break;
    //     default:
    //     break;
    // }
    // return ret;
}

// TODO: shouldn't port be unsigned short?

int setupSocketAndReturnDescriptor(const char * serverAddressString, int serverPort)
{
        int socketFileDescriptor;


    struct sockaddr_in serverAddressStruct;
    struct hostent *server;


    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor < 0)
    {
        return SOCKET_OPEN_FAILURE;
    }
    cout << socketFileDescriptor<<endl;
    server = gethostbyname(serverAddressString);

    if (server == NULL) {

       return SOCKET_UNKNOWN_HOST;

    }
cout << server << endl;
    memset((char *) &serverAddressStruct, 0,sizeof(serverAddressStruct));

    serverAddressStruct.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serverAddressStruct.sin_addr.s_addr,
         server->h_length);
    serverAddressStruct.sin_port = htons(serverPort);

   // cout << "HOST: "<<serverAddressString << " PORT: " << serverPort<<endl;

    if (connect(socketFileDescriptor,(struct sockaddr *) &serverAddressStruct,sizeof(serverAddressStruct)) < 0)
    {
        return SOCKET_CONNECTION_FALIURE;
    }
    cout << 'A'<<endl;

    return socketFileDescriptor;
}
int setupSocketAndReturnDescriptor(const char * serverAddressString, char * serverPortString)
{
    int serverPort = atoi(serverPortString);
    return setupSocketAndReturnDescriptor(serverAddressString, serverPort);
}

int createSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

int listenOnSocket(int localSocketFd) {
    struct sockaddr_in binderAddress;
    memset((struct sockaddr_in *)&binderAddress, 0, sizeof(binderAddress));

    binderAddress.sin_family = AF_INET;
    binderAddress.sin_addr.s_addr = INADDR_ANY;
    binderAddress.sin_port = 0;

    if (bind(localSocketFd, (struct sockaddr *) &binderAddress, sizeof(binderAddress)) < 0)
    {
        cerr << "ERROR: Failed to bind on local socket"<<endl;
        return SOCKET_LOCAL_BIND_FALIURE;
    }

    listen(localSocketFd, 5);
}

int acceptConnection(int localSocketFd) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int newSocketFd = accept(localSocketFd, (struct sockaddr *) &clientAddress, &clientAddressSize);

    if (newSocketFd < 0)
    {
        cerr<<"ERROR: Failed to accept client connection"<<endl;
        return SOCKET_ACCEPT_CLIENT_FAILURE;
    }
        

    return newSocketFd;
}

unsigned short getPort(int localSocketFd) {
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

void printSettings(int localSocketFd) {
    char localHostName[256];
    gethostname(localHostName, 256);
    cout << "BINDER_ADDRESS " << localHostName << endl;

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(localSocketFd, (struct sockaddr *)&sin, &len);
    cout << "BINDER_PORT " << ntohs(sin.sin_port) << endl;
}


// Client Server helpers






int insertClientServerMessageToBuffer(char *messagePointer, const char* name, int* argTypes, void**args)
{
    RWBuffer b;
    unsigned int argTypesLength = b.returnArgTypesLength(argTypes);
    messagePointer = b.insertUnsignedIntToBuffer(strlen(name)+1, messagePointer);
    messagePointer = b.insertCStringToBuffer(name, messagePointer);
    messagePointer = b.insertIntArrayToBuffer(argTypes, argTypesLength, messagePointer);

    for(unsigned int i = 0; i < argTypesLength-1; i++)
    {
        int argType = argTypes[i];

        unsigned short length = b.getArrayLengthFromArgumentType(argType);
        if(length == 0) length= 1; // treat scalars and arrays of length 1 the same

        int type = b.getTypeFromArgumentType(argType);

        void * arg = args[i];

        switch(type)
        {
            case ARG_CHAR:
            {
                char * chars = (char *)arg;
                if(debug) cout << " "<< i<<": Char ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << chars[j]<< " ";
                    messagePointer = b.insertCharToBuffer(chars[j], messagePointer);
                }

                if(debug) cout << endl;
            }

            break;
            case ARG_SHORT:
            {
                short * shorts = (short *)arg;
                if(debug) cout << " "<< i<<": Short ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << shorts[j] << " ";
                    messagePointer = b.insertShortToBuffer(shorts[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            break;
            case ARG_INT:
            {
                int * ints = (int *)arg;
                if(debug) cout << " "<< i<<": Int ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << ints[j] << " ";
                    messagePointer = b.insertIntToBuffer(ints[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            case ARG_LONG:
            {
                long * longs = (long *)arg;
                if(debug) cout << " "<< i<<": Long "<<length<<" ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << longs[j] << " ";
                    messagePointer = b.insertLongToBuffer(longs[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            case ARG_DOUBLE:
            {
                double * doubles = (double *)arg;
                if(debug) cout << " "<< i<<": Double ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << doubles[j] << " ";
                    messagePointer = b.insertDoubleToBuffer(doubles[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            case ARG_FLOAT:
            {
                float * floats = (float *)arg;
                if(debug) cout << " "<< i<<": Float ";
                for(int j = 0; j < length; j++)
                {
                    if(debug) cout << floats[j] << " ";
                    messagePointer = b.insertFloatToBuffer(floats[j], messagePointer);
                }
                if(debug) cout << endl;
            }
            break;
            break;
            default:
                cerr << "WARNING: argument of unknown type."<<endl;
            break;
        }
    }
    return 0;
}
unsigned int getClientServerMessageLength(const char* name, int* argTypes, void**args)
{
    unsigned int argTypesLength = 0;
    unsigned int messageSize = 0;

    // calculate length of arguments
    int * argTypesP = argTypes;
    RWBuffer b;
    while(*argTypesP != 0)
    {
        int argType = *argTypesP;

        unsigned short length = b.getArrayLengthFromArgumentType(argType);
        if(length == 0) length = 1; // if it's a scalar, it still takes up one room

        int type = b.getTypeFromArgumentType(argType);
        unsigned int size = sizeOfType(type);
        messageSize += length*size;

        argTypesP++;
        argTypesLength++;
    }
    argTypesLength++; //accout for the 0

    // calculate length of argTypes
    messageSize += 4*argTypesLength;

    // calculate name size
    const char * nameP = name;
    while(*nameP != '\0')
    {
        messageSize++;
        nameP++;
    }
    messageSize++; //accountfor null termination char

    messageSize += 4; // account for sending size of name

    return messageSize;
}
int cleanupArgumentsMessage(char * bufferPointer, int argTypes[], void * args[], unsigned int argTypesLength)
{
    RWBuffer b;
    bufferPointer = b.extractArgTypes(bufferPointer, argTypes);


    for(unsigned int i = 0; i < argTypesLength-1 /*ignores the last (0) int*/; i++)
    {
        int argType = argTypes[i];
        unsigned short int length = b.getArrayLengthFromArgumentType(argType);
        int type = b.getTypeFromArgumentType(argType);
        if(length==0)
        {
            delete args[i];
        }
        else
        {
            delete [] args[i];
        }
    }
    cout << "Cleanup done"<<endl;
    return 0;
}
int extractArgumentsMessage(char * bufferPointer, int argTypes[], void * args[], unsigned int argTypesLength, bool allocateMemory)
{
    RWBuffer b;
    bufferPointer = b.extractArgTypes(bufferPointer, argTypes);


    for(unsigned int i = 0; i < argTypesLength-1 /*ignores the last (0) int*/; i++)
    {
        int argType = argTypes[i];
        unsigned short int length = b.getArrayLengthFromArgumentType(argType);
        int type = b.getTypeFromArgumentType(argType);

        if(allocateMemory)
        {


            switch(type)
            {
                case ARG_CHAR:
                {
                    if(length == 0)
                    {

                        char * c = new char();
                        bufferPointer = b.extractChar(bufferPointer, *c);

                        args[i] = (void *)c;

                        if(debug) cout <<" "<<i<< ": "<< "Char "<< *c<<endl;
                    }
                    else
                    {
                        char * cArray = new char[length];
                        bufferPointer = b.extractCharArray(bufferPointer, cArray, length);
                        args[i] = (void *)cArray;

                        if(debug){

                            cout <<" "<<i<< ": "<< "Chars ";
                            for(int j = 0; j < length; j++)
                            {
                                cout << cArray[j] << " ";
                            }
                            cout << endl;
                        }
                    }

                }
                break;
                case ARG_SHORT:
                {
                    if(length == 0)
                    {
                        short * s = new short();
                        bufferPointer = b.extractShort(bufferPointer, *s);

                        args[i] = (void *)s;

                        if(debug) cout <<" "<<i<< ": "<< "Short "<< *s<<endl;
                    }
                    else
                    {
                        short * sArray = new short[length];
                        bufferPointer = b.extractShortArray(bufferPointer, sArray, length);
                        args[i] = (void *)sArray;

                        if(debug){

                            cout <<" "<<i<< ": "<< "Shorts ";
                            for(int j = 0; j < length; j++)
                            {
                                cout << sArray[j] << " ";
                            }
                            cout << endl;
                        }
                    }
                }
                break;
                case ARG_INT:
                {
                    if(length == 0)
                    {
                        int * c = new int();
                        bufferPointer = b.extractInt(bufferPointer, *c);

                        args[i] = (void *)c;

                        if(debug) cout <<" "<<i<< ": "<< "Int "<< *c<<endl;
                    }
                    else
                    {
                        int * cs = new int[length];
                        bufferPointer = b.extractIntArray(bufferPointer, cs, length);
                        args[i] = (void *)cs;

                        if(debug){

                            cout <<" "<<i<< ": "<< "Ints ";
                            for(int j = 0; j < length; j++)
                            {
                                cout << cs[j] << " ";
                            }
                            cout << endl;
                        }
                    }

                }
                break;
                case ARG_LONG:
                {
                    if(length == 0)
                    {
                        long * l = new long();
                        bufferPointer = b.extractLong(bufferPointer, *l);

                        args[i] = (void *)l;

                        if(debug) cout <<" "<<i<< ": "<< "Long "<< *l<<endl;


                    }
                    else
                    {
                        long * lArray = new long[length];
                        bufferPointer = b.extractLongArray(bufferPointer, lArray, length);
                        args[i] = (void *)lArray;

                        if(debug){

                            cout <<" "<<i<< ": "<< "Longs ";
                            for(int j = 0; j < length; j++)
                            {
                                cout << lArray[j] << " ";
                            }
                            cout << endl;
                        }
                    }
                }
                break;
                case ARG_DOUBLE:
                {
                    if(length == 0)
                    {
                        double * d = new double();
                        bufferPointer = b.extractDouble(bufferPointer, *d);

                        args[i] = (void *)d;

                        if(debug) cout <<" "<<i<< ": "<< "Double "<< *d<<endl;
                    }
                    else
                    {
                        double * dArray = new double[length];
                        bufferPointer = b.extractDoubleArray(bufferPointer, dArray, length);
                        args[i] = (void *)dArray;

                        if(debug){

                            cout <<" "<<i<< ": "<< "Doubles ";
                            for(int j = 0; j < length; j++)
                            {
                                cout << dArray[j] << " ";
                            }
                            cout << endl;
                        }
                    }
                }
                break;
                case ARG_FLOAT:
                {
                    if(length == 0)
                    {
                        float * f = new float();
                        bufferPointer = b.extractFloat(bufferPointer, *f);

                        args[i] = (void *)f;

                        if(debug) cout <<" "<<i<< ": "<< "Float "<< *f<<endl;
                    }
                    else
                    {
                        float * fArray = new float[length];
                        bufferPointer = b.extractFloatArray(bufferPointer, fArray, length);
                        args[i] = (void *)fArray;

                        if(debug){

                            cout <<" "<<i<< ": "<< "Floats ";
                            for(int j = 0; j < length; j++)
                            {
                                cout << fArray[j] << " ";
                            }
                            cout << endl;
                        }
                    }
                }
                break;
                default:
                if(debug) cout << "WARNING: Argument has unknown type"<<endl;
                break;
            }
        }
        else
        {
             switch(type)
        {
            case ARG_CHAR:
            {
                if(length == 0)
                {


                    bufferPointer = b.extractChar(bufferPointer, *((char *)args[i]));


                    //if(debug) cout <<" "<<i<< ": "<< "Char "<< *(args[i])<<endl;
                }
                else
                {

                    bufferPointer = b.extractCharArray(bufferPointer, (char *)args[i], length);


                   /* if(debug){

                        cout <<" "<<i<< ": "<< "Chars ";
                        for(int j = 0; j < length; j++)
                        {
                            cout << args[i][j] << " ";
                        }
                        cout << endl;
                    }*/
                }

            }
            break;
            case ARG_SHORT:
            {
                if(length == 0)
                {
                    short * s = new short();
                    bufferPointer = b.extractShort(bufferPointer, *((short *)args[i]));


                   // if(debug) cout <<" "<<i<< ": "<< "Short "<< *s<<endl;
                }
                else
                {

                    bufferPointer = b.extractShortArray(bufferPointer, (short *)args[i], length);

                    /*if(debug){

                        cout <<" "<<i<< ": "<< "Shorts ";
                        for(int j = 0; j < length; j++)
                        {
                            cout << args[i][j] << " ";
                        }
                        cout << endl;
                    }*/
                }
            }
            break;
            case ARG_INT:
            {
                if(length == 0)
                {

                    bufferPointer = b.extractInt(bufferPointer, *((int *)args[i]));



                    //if(debug) cout <<" "<<i<< ": "<< "Int "<< <<endl;
                }
                else
                {

                    bufferPointer = b.extractIntArray(bufferPointer, (int *)args[i], length);


                    /*if(debug){

                        cout <<" "<<i<< ": "<< "Ints ";
                        for(int j = 0; j < length; j++)
                        {
                            cout << cs[j] << " ";
                        }
                        cout << endl;
                    }*/
                }

            }
            break;
            case ARG_LONG:
            {
                if(length == 0)
                {

                    bufferPointer = b.extractLong(bufferPointer, *((long *)args[i]));



                  //  if(debug) cout <<" "<<i<< ": "<< "Long "<< *l<<endl;


                }
                else
                {
                    //long * lArray = new long[length];
                    bufferPointer = b.extractLongArray(bufferPointer, (long *)args[i], length);
                    //args[i] = (void *)lArray;

                    /*if(debug){

                        cout <<" "<<i<< ": "<< "Longs ";
                        for(int j = 0; j < length; j++)
                        {
                            cout << lArray[j] << " ";
                        }
                        cout << endl;
                    }*/
                }
            }
            break;
            case ARG_DOUBLE:
            {
                if(length == 0)
                {
                    //double * d = new double();
                    bufferPointer = b.extractDouble(bufferPointer, *((double *)args[i]));

                    //args[i] = (void *)d;

                    //if(debug) cout <<" "<<i<< ": "<< "Double "<< *d<<endl;
                }
                else
                {
                    //double * dArray = new double[length];
                    bufferPointer = b.extractDoubleArray(bufferPointer, (double *)args[i], length);
                    //args[i] = (void *)dArray;

                   /* if(debug){

                        cout <<" "<<i<< ": "<< "Doubles ";
                        for(int j = 0; j < length; j++)
                        {
                            cout << dArray[j] << " ";
                        }
                        cout << endl;
                    }*/
                }
            }
            break;
            case ARG_FLOAT:
            {
                if(length == 0)
                {
                    //float * f = new float();
                    bufferPointer = b.extractFloat(bufferPointer, *((float *)args[i]));

                    //args[i] = (void *)f;

                    //if(debug) cout <<" "<<i<< ": "<< "Float "<< *f<<endl;
                }
                else
                {
                   // float * fArray = new float[length];
                    bufferPointer = b.extractFloatArray(bufferPointer, (float *)args[i], length);
                   // args[i] = (void *)fArray;

                   /* if(debug){

                        cout <<" "<<i<< ": "<< "Floats ";
                        for(int j = 0; j < length; j++)
                        {
                            cout << fArray[j] << " ";
                        }
                        cout << endl;
                    }*/
                }
            }
            break;
            default:
            if(debug) cout << "WARNING: Argument has unknown type"<<endl;
            break;
        }
        }
    }
    cout << "DONE";
    return 0;
}