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
#include "helpers.h"
#include <map>
#include <pthread.h>
#include <vector>

using namespace std;
static bool debug = true;
static int localSocketFd;
static int binderSocketFd;
static pthread_mutex_t * _threadLock;
static map<pthread_t, bool> _runningThreads;
static bool shouldTerminate = false;
static map<rpcFunctionKey, skeleton> registeredFunctions;

/*bool operator > (const pthread_t& left, const pthread_t& right)
{
return left.p > right.p;
}*/

int rpcInit() {
    char * binderAddressString = getenv ("BINDER_ADDRESS");
    char * binderPortString = getenv("BINDER_PORT");

    if(binderAddressString == NULL)
	{
		return INIT_UNSET_BINDER_ADDRESS;
	}
    if(binderPortString == NULL)
	{
		return INIT_UNSET_BINDER_PORT;
	}

    binderSocketFd = setupSocketAndReturnDescriptor(binderAddressString, binderPortString);

	signal(SIGPIPE, SIG_IGN);
    if (binderSocketFd < 0) {
        return INIT_BINDER_SOCKET_FAILURE;
    }

    localSocketFd = createSocket();
    listenOnSocket(localSocketFd);

    if (localSocketFd < 0) {
        return INIT_LOCAL_SOCKET_FAILURE;
    }

    _threadLock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	 pthread_mutex_init(_threadLock, NULL);

    return 0;
}

// returns 0 on sucess
// warnings:
// positive returned
// errors:
// negative returned
int rpcRegister(char *name, int *argTypes, skeleton f) {
	// calls the binder, informing it that a server procedure
	// with the indicated name and list of argument types is available at this server.

	// The result returned is 0 for a successful registration, positive for a warning
	// (e.g., this is the same as some previously registered procedure), or negative
	// for failure (e.g., could not locate binder).

	// The function also makes an entry in a local database,
	// associating the server skeleton with the name and list of argument types.
	string hostname = getHostname();
	unsigned short port = getPort(localSocketFd);
	string funcName = string(name);

	Sender s(binderSocketFd);

	int argTypesLength = 0;
	while (argTypes[argTypesLength++]);

	int n = s.sendRegisterMessage(hostname, port, funcName, argTypes);
	if(n < 0) return n;

	// listen for response
	Receiver r(binderSocketFd);
	unsigned int messageSize;
	n = r.receiveMessageSize(messageSize);
	if(n < 0) return n;

	MessageType type;
	n = r.receiveMessageType(type);
	if(n < 0) return n;



	if(type == REGISTER_SUCCESS)
	{
		int reasonCode = 0;
		if(	messageSize > 0)
		{
			char buffer[messageSize];
			n = r.receiveMessageGivenSize(messageSize, buffer);
			if(n < 0) return n;
			RWBuffer b;
			b.extractInt(buffer, reasonCode);
		}

		struct rpcFunctionKey k(string(name), argTypes);
		registeredFunctions[k] = f;

		return reasonCode;
	}
	else if(type == REGISTER_FAILURE)
	{
		int errorCode = 0;
		if(	messageSize > 0)
		{
			char buffer[messageSize];
			n = r.receiveMessageGivenSize(messageSize, buffer);
			if(n < 0) return n;
						RWBuffer b;
			b.extractInt(buffer, errorCode);
		}

		return errorCode;
	}
	else
	{
		return RECEIVE_INVALID_MESSAGE_TYPE;
	}

	return 0;
}
void removeThreadFromList(pthread_t key)
{

	if(_threadLock)
	{
		pthread_mutex_lock(_threadLock);
		_runningThreads.erase(key);
		pthread_mutex_unlock(_threadLock);
	}
	// else no lock? Don't do anything. Unsafe.
}
void handleExecuteMessage(int clientSocketFd,char * message, unsigned int messageSize)
{
	RWBuffer b;

	char * bufferPointer = message;

	unsigned int nameSize;
	bufferPointer = b.extractUnsignedInt(bufferPointer, nameSize);
	char nameChar[nameSize];

	bufferPointer = b.extractCharArray(bufferPointer, nameChar, nameSize);
	string name(nameChar);


	unsigned int argTypesLength = b.returnArgTypesLength(bufferPointer);
	int argTypes[argTypesLength];
	void * args[argTypesLength];

	extractArgumentsMessage(bufferPointer, argTypes, args, argTypesLength, true);

	struct rpcFunctionKey k(name, argTypes);
	skeleton skel = registeredFunctions[k];

	int failCode = 0;

	Sender s(clientSocketFd);

	if(skel == NULL){
		// error, function not found. send fail message
		failCode = EXECUTE_UNKNOWN_SKELETON;
	}
	else
	{
		// call the skeleton
		int r = skel(argTypes, args);
		if(r == 0)
		{
			// send success message
			char returnMessage[messageSize];

			insertClientServerMessageToBuffer(returnMessage, name.c_str(), argTypes, args);
		    int r = s.sendMessage(messageSize, EXECUTE_SUCCESS, returnMessage);
		}
		else
		{
			// send fail message
			failCode = r;
		}
	}

	if(failCode != 0)
	{
		// send fail message
		char failureCodeMessage[4];
		b.insertIntToBuffer(failCode, failureCodeMessage);
		int r = s.sendMessage(4, EXECUTE_FAILURE, failureCodeMessage);
	}

	cleanupArgumentsMessage(bufferPointer, argTypes, args, argTypesLength);

}

void * handleExecuteMessageThreadFunction(void * args)
{
	// thread starts
	void ** argArray = (void **)args;
	int * clientSocketFd = ((int *)argArray[0]);
	unsigned int * messageSize = ((unsigned int *)argArray[1]);
	char * buffer = (char *)argArray[2];

	handleExecuteMessage(*clientSocketFd,buffer, *messageSize);


	// deallocate arguments
	delete clientSocketFd;
	delete messageSize;
	delete [] buffer;
	// thread finishes

	removeThreadFromList(pthread_self());

	return NULL;
}
int handleRequest(int clientSocketFd, fd_set *master_set, map<int, unsigned int> &chunkInfo) {
	Receiver receiver(clientSocketFd);
    bool success = false;

	unsigned int messageSize;

    if(receiver.receiveMessageSize(messageSize) == 0 )
    {
        MessageType type;
        if(receiver.receiveMessageType(type) == 0)
        {
        	 if (binderSocketFd != clientSocketFd && type == EXECUTE) {

		        char buffer[messageSize];
		        if (receiver.receiveMessageGivenSize(messageSize, buffer) == 0)
		        {

		        	// TODO: handle in another thread in the future
		        	char * bufferCopy = new char[messageSize];
		        	unsigned int * messageSizeCopy = new unsigned int(messageSize);
		        	int * clientSocketFdCopy = new int(clientSocketFd);
		        	void ** threadArguments = new void*[3];

		        	// copy memory over
		        	memcpy ( bufferCopy, buffer, messageSize);

		        	threadArguments[0] = (void *)clientSocketFdCopy;
		        	threadArguments[1] = (void *)messageSizeCopy;
		        	threadArguments[2] = (void *)bufferCopy;

		        	pthread_t sendingThread ;
    				pthread_create(&sendingThread, NULL, &handleExecuteMessageThreadFunction, (void *)threadArguments);
    				_runningThreads[sendingThread] = true;

		            success = true;
		        }
		    }
		    else if(binderSocketFd == clientSocketFd && type == TERMINATE)
		    {
		    	// we received a terminate message from the binder, so we kill ourself
		    	return -1;
		    }
        }
    }

    if (!success) {
        chunkInfo[clientSocketFd] = 0;
        FD_CLR(clientSocketFd, master_set);
        close(clientSocketFd);

        if(clientSocketFd == binderSocketFd)
        {
        	// binder closed, so we should termiante as well
        	return -1;
        }

    }
    return 0;
}
int rpcExecute()
{
	if(registeredFunctions.size() == 0) return -1; // no functions to serve
    printSettings(localSocketFd);

    int max_fd = localSocketFd;
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    FD_SET(localSocketFd, &master_set);
    FD_SET(binderSocketFd, &master_set);
    map<int, unsigned int> chunkInfo;

    while (!shouldTerminate) {

    		memcpy(&working_set, &master_set, sizeof(master_set));
	        int selectResult = select(max_fd + 1, &working_set, NULL, NULL, NULL);

	        if (selectResult < 0) {
	            return SELECT_FAILED;
	        } else if (selectResult == 0) {
	            return SELECT_TIMED_OUT;
	        }

	        for (int i = 0; i < max_fd + 1; i++) {
	            if (FD_ISSET(i, &working_set)) {
	                if (i != localSocketFd) {
	                    int clientSocketFd = i;
	                    int requestCode = handleRequest(clientSocketFd, &master_set, chunkInfo);
	                    if(requestCode < 0)
	                    {
	                    	// received a termination signal from binder
	                    	// break out of the execute loop
	                    	shouldTerminate = true;
	                    	break;

	                    }
	                } else {
	                    int newSocketFd = acceptConnection(localSocketFd);
	                    if(newSocketFd > max_fd) max_fd = newSocketFd;
	                    FD_SET(newSocketFd, &master_set);
	                }
	            }
	        }


    }

    vector<pthread_t> livingThreads;

    // make a copy of all living threads while within the lock
    pthread_mutex_lock(_threadLock);
    map<pthread_t, bool>::iterator existingThreadsBegin = _runningThreads.begin();
    while(existingThreadsBegin != _runningThreads.end())
    {
    	livingThreads.push_back(existingThreadsBegin->first);
    	existingThreadsBegin++;
    }
	pthread_mutex_unlock(_threadLock);

	// wait until the list of living threads are all done
	for(int i = 0; i < livingThreads.size(); i++)
	{
		pthread_join(livingThreads[i], NULL);
	}


    close(binderSocketFd);
    close(localSocketFd);

    pthread_mutex_destroy(_threadLock);
	free(_threadLock);
	return 0;
}
