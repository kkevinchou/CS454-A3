#include "rpc.h"
#include "helpers.h"
#include <stdlib.h> //getenv
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <map>
#include "sender.h"
#include "constants.h"
#include "receiver.h"
#include "rwbuffer.h"
using namespace std;

static bool debug = true;
static int binderSocketFd = -1;

int connectToBinder() {
    if (binderSocketFd >= 0) {
        return 0;
    }

    char * binderAddressString = getenv ("BINDER_ADDRESS");
    char * binderPortString = getenv("BINDER_PORT");

    if(binderAddressString == NULL)
    {
        cerr << "ERROR: BINDER_ADDRESS environment variable not set."<< endl;
        return INIT_UNSET_BINDER_ADDRESS;
    }
    if(binderPortString == NULL)
    {
        cerr << "ERROR: BINDER_PORT environment variable not set." << endl;
        return INIT_UNSET_BINDER_PORT;
    }
    cerr << "connecting to : " << binderAddressString << ":" << binderPortString << endl;
    binderSocketFd = setupSocketAndReturnDescriptor(binderAddressString, binderPortString);

    if (binderSocketFd < 0) {
        return INIT_BINDER_SOCKET_FAILURE;
    }

    return 0;
}

int sendLocRequest(string name, int argTypes[]) {
    int status = connectToBinder();

    if (status == 0) {
        Sender s(binderSocketFd);
        status = s.sendLocRequestMessage(name, argTypes);
    }

    return status;
}


int sendExecuteRequest(int serverSocketFd,char* name, int* argTypes, void**args)
{
    RWBuffer b;
    Sender s(serverSocketFd);

    unsigned int messageSize = getClientServerMessageLength(name, argTypes, args);



    if(debug)
    {
        cout << endl;
        cout << "Name: " << name<<endl;
    }
    // create buffer for full message
    char message[messageSize];

   // char * messagePointer = message;
  //  messagePointer = b.insertUnsignedIntToBuffer(messageSize, messagePointer);
   // messagePointer = b.insertIntToBuffer(static_cast<int>(EXECUTE), messagePointer);

   insertClientServerMessageToBuffer(message, name, argTypes, args);

    if(debug)
    {
        cout << "Sending an EXECUTE message of size "<<messageSize << ": "<<endl;
       /* for(unsigned int i = 0; i < messageSize; i++)
        {
            cout << (int)message[i] << " ";

        }*/
        cout << endl;
    }

    // send remote procedure call
    int sendResult = s.sendMessage(messageSize, EXECUTE, message);
    if(sendResult != 0) return sendResult;
    if(debug) cout << " ...Listening for reply..."<<endl;
    // listen for a reply
    Receiver r(serverSocketFd);
    int n = r.receiveMessageSize(messageSize);
    if(n < 0) return n;
    MessageType type;
    n = r.receiveMessageType(type);
    if(n < 0) return n;
    if(debug)cout << "Received "<< messageSize << " "<<type<<endl;

    char replyMessage[messageSize];
    n = r.receiveMessageGivenSize(messageSize, replyMessage);
    if(n < 0) return n;

    int returnCode = 0;
    switch(type)
    {
        case EXECUTE_SUCCESS:
        {
            // extract argument data
            if(debug)cout << "Success!"<<endl;
            char * replyMessageP = replyMessage;
            unsigned int argTypesLength = b.returnArgTypesLength(argTypes);
            string functionName;
            replyMessageP = b.extractString(replyMessageP, functionName);
            if(strcmp(functionName.c_str(), name) == 0)
            {
                extractArgumentsMessage(replyMessageP, argTypes, args, argTypesLength, false);
            }
            else
            {
                returnCode = WRONG_FUNCTION_NAME_RETURNED;
                cerr << "WARNING: wrong function name returned: " << functionName<<endl;
            }


        }
        break;
        case EXECUTE_FAILURE:
        {
            // extract error code
            if(debug)cout << "Failure"<<endl;
            b.extractInt(replyMessage, returnCode);
        }
        break;
        default:
            cerr << "WARNING: Server returned an invalid response to remote prodcure call."<<endl;
            returnCode = RECEIVE_INVALID_MESSAGE_TYPE;
        break;
    }


    return returnCode;
}

int processLocResponse(string &serverID, unsigned short &port) {
    Receiver r(binderSocketFd);

    unsigned int messageSize;

    int n = r.receiveMessageSize(messageSize);
    if(n < 0) return n;

    MessageType msgType;
    n = r.receiveMessageType(msgType);
    if(n < 0) return n;

    char buffer[messageSize];
    char *bufferPointer = buffer;
    n = r.receiveMessageGivenSize(messageSize, buffer);
    if(n<0) return n;
    RWBuffer b;

    if (msgType == LOC_SUCCESS) {
        bufferPointer = b.extractString(bufferPointer, serverID);
        bufferPointer = b.extractUnsignedShort(bufferPointer, port);
        return 0;
    } else if (msgType == LOC_FAILURE) {
        cerr << "FAILURE" << endl;
        int reasonCode = 0;
        if(messageSize > 0)
        {

            b.extractInt(buffer, reasonCode);
        }
        return reasonCode;
    } else {
        cerr << "UNEXPECTED MSGTYPE IN processLocResponse()" << endl;
        return RECEIVE_INVALID_MESSAGE_TYPE;
    }
}

int rpcCall(char* name, int* argTypes, void** args) {
    // cerr << "RPC CALL" << endl;
    int locRequestCode = sendLocRequest(string(name), argTypes);

    if(locRequestCode < 0) return locRequestCode;

    string serverID;
    unsigned short port;
    int locCode = processLocResponse(serverID, port);

    if (locCode < 0) {
        return locCode;
    }

  //  cerr << "SERVER : " << serverID << endl;
  //  cerr << "PORT : " << port << endl;
    // char * binderAddressString = getenv ("BINDER_ADDRESS");
    // char * binderPortString = getenv("BINDER_PORT");
    int serverSocketFd = setupSocketAndReturnDescriptor(serverID.c_str(), port);

    int n = sendExecuteRequest(serverSocketFd,name, argTypes, args);

    close(serverSocketFd);
    return n;
}
int rpcTerminate()
{
    int status = 0;

    if (binderSocketFd < 0) {
        status = connectToBinder();
    }

    if (status == 0) {
        Sender s(binderSocketFd);
        status = s.sendTerminateMessage();
    }

    return status;
}