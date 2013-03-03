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
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "rwbuffer.h"

using namespace std;

static int localSocketFd;
static int binderSocketFd;

static map<rpcFunctionKey, skeleton> registeredFunctions;

int rpcInit() {
    char * binderAddressString = getenv ("BINDER_ADDRESS");
    char * binderPortString = getenv("BINDER_PORT");

    if(binderAddressString == NULL) error("ERROR: BINDER_ADDRESS environment variable not set.");
    if(binderPortString == NULL) error("ERROR: BINDER_PORT environment variable not set.");

    cerr << "connecting to : " << binderAddressString << ":" << binderPortString << endl;
    binderSocketFd = setupSocketAndReturnDescriptor(binderAddressString, binderPortString);

	signal(SIGPIPE, SIG_IGN);
    if (binderSocketFd < 0) {
        return binderSocketFd;
    }

    localSocketFd = createSocket();

    if (localSocketFd < 0) {
        return localSocketFd;
    }

    return 0;
}

// returns 0 on sucess
// warnings:
// 1 - function already added
// errors:
// -1 - binder could not be found
int rpcRegister(char *name, int *argTypes, skeleton f) {
	// calls the binder, informing it that a server procedure
	// with the indicated name and list of argument types is available at this server.

	// The result returned is 0 for a successful registration, positive for a warning
	// (e.g., this is the same as some previously registered procedure), or negative
	// for failure (e.g., could not locate binder).

	// The function also makes an entry in a local database,
	// associating the server skeleton with the name and list of argument types.
	struct rpcFunctionKey k(string(name), argTypes);

	if(registeredFunctions[k] != NULL) return 1;
	registeredFunctions[k] = f;

	string hostname = getHostname();
	short port = getPort(localSocketFd);
	string funcName = string(name);

	Sender s(binderSocketFd);

	int argTypesLength = 0;
	while (argTypes[argTypesLength++]);

	cerr << "hostname : " << hostname.size() << endl;
	cerr << "funcName : " << funcName.size() << endl;
	cerr << "argTypesLength : " << argTypesLength << endl;

	s.sendRegisterMessage(hostname, port, funcName, argTypes);

	cout << "Successfully registered: "<< name << endl;

	return 0;
}
void printSettings(int localSocketFd) {
    char localHostName[256];
    gethostname(localHostName, 256);
    cout << "SERVER_ADDRESS " << localHostName << endl;

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(localSocketFd, (struct sockaddr *)&sin, &len);
    cout << "SERVER_PORT " << ntohs(sin.sin_port) << endl;
}


void handleExecuteMessage(char * message, unsigned int messageSize)
{
	bool debug = true;
	if(debug){
		cout << endl;
		cout << "Received an EXECUTE message of size "<<messageSize<<": "<<endl;
	}
	for(int i = 0; i < messageSize; i++)
	{
		cout << (int)message[i]<<" ";
	}
	cout << endl;
	RWBuffer b;
 // string recvStr(buffer, buffer+size);
	// cout << recvStr << endl;

	// Sender s(clientSocketFd);
	// s.sendMessage(recvStr);
	string name;


	char * bufferPointer = message;
	bufferPointer = b.extractString(bufferPointer, name);
	if(debug)cout << "Name: "<<name<<endl;

	unsigned int argTypesLength = b.returnArgTypesLength(bufferPointer);

	int argTypes[argTypesLength];
	bufferPointer = b.extractArgTypes(bufferPointer, argTypes);

	void * args[argTypesLength];

	for(int i = 0; i < argTypesLength; i++)
	{
		int argType = argTypes[i];
		unsigned short int length = b.getArrayLengthFromArgumentType(argType);
		int type = b.getTypeFromArgumentType(argType);

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
			break;
		}
	}
}


void handleRequest(int clientSocketFd, fd_set *master_set, map<int, unsigned int> &chunkInfo) {
	Receiver receiver(clientSocketFd);
    bool success = false;

// if (chunkInfo[clientSocketFd] == 0) {
	unsigned int messageSize;

     //   cout << "nb" << nb << " " << numBytes<<endl;
    if(receiver.receiveMessageSize(messageSize) == 0 )
    {
        // TODO : HANDLE TERMINATE
        MessageType type = receiver.receiveMessageType();

        if (type == EXECUTE) {
           // cout << "Received execute message"<<endl;


	        char buffer[messageSize];
	        if (receiver.receiveMessageGivenSize(messageSize, buffer) == 0)
	        {

	        	// TODO: handle in another thread in the future
	           	handleExecuteMessage(buffer, messageSize);


	            success = true;
	        }
	    }

    }

    if (!success) {
        chunkInfo[clientSocketFd] = 0;
        FD_CLR(clientSocketFd, master_set);
        close(clientSocketFd);
    }

}
int rpcExecute()
{
	if(registeredFunctions.size() == 0) return -1; // no functions to serve

	listenOnSocket(localSocketFd);
    printSettings(localSocketFd);

    int max_fd = localSocketFd;
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    FD_SET(localSocketFd, &master_set);

    map<int, unsigned int> chunkInfo;

    while (true) {
        memcpy(&working_set, &master_set, sizeof(master_set));
        int selectResult = select(max_fd + 1, &working_set, NULL, NULL, NULL);

        if (selectResult < 0) {
            error("ERROR: Select failed");
        } else if (selectResult == 0) {
            error("ERROR: Select timed out");
        }

        for (int i = 0; i < max_fd + 1; i++) {
            if (FD_ISSET(i, &working_set)) {
                if (i != localSocketFd) {
                    int clientSocketFd = i;
                    handleRequest(clientSocketFd, &master_set, chunkInfo);
                } else {
                    cout << "accept connection"<<endl;
                    int newSocketFd = acceptConnection(localSocketFd);
                    if(newSocketFd > max_fd) max_fd = newSocketFd;
                    FD_SET(newSocketFd, &master_set);
                }
            }
        }
    }

    close(localSocketFd);
	return 0;
}
