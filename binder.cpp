#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <list>
#include <strings.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <sstream>
#include <signal.h>
#include <map>
#include "sender.h"
#include "receiver.h"
#include "helpers.h"
#include "constants.h"
#include "rpc.h" // for the typedef
#include "rwbuffer.h"

using namespace std;

static map<rpcFunctionKey, list<server_info *>* > servicesDictionary;
// static map<int, server_info>
static map<int, unsigned int> chunkInfo;
static map<int, MessageType> msgInfo;

extern bool debug;

void addService(string name, int argTypes[], string serverID, unsigned short port) {
    rpcFunctionKey key(name, argTypes);
    server_info location(serverID, port);

    if (servicesDictionary.find(key) == servicesDictionary.end()) {
        // The key doesn't exist.  Since argTypes is allocated on the stack, we
        // need to allocate memory for the key on the heap.
        unsigned int argTypesLength = 0;
        while (argTypes[argTypesLength++]);
        int *memArgTypes = new int[argTypesLength];

        unsigned int i = 0;
        while (argTypes[i] != 0) {
            memArgTypes[i] = argTypes[i];
            i++;
        }
        memArgTypes[i] = 0;

        key = rpcFunctionKey(name, memArgTypes);
        servicesDictionary[key] = new list<server_info *>();
        cerr << "NEW KEY" << endl;
    } else {
        cerr << "OLD KEY" << endl;
    }

    list<server_info *> *hostList = servicesDictionary[key];
    server_info *l = new server_info(location);

    for (list<server_info *>::iterator it = hostList->begin(); it != hostList->end(); it++) {
        server_info *oldServerInfo = *it;
        if (*oldServerInfo == *l) {
            hostList->remove(oldServerInfo);
            delete oldServerInfo;
        }
    }

    hostList->push_back(l);
}

void handleRegisterRequest(Receiver &receiver, char buffer[], unsigned int bufferSize) {
    string serverID;
    unsigned short port;
    string name;
    RWBuffer b;

    char * bufferPointer = buffer;
    bufferPointer = b.extractString(bufferPointer, serverID);
    bufferPointer = b.extractUnsignedShort(bufferPointer, port);
    bufferPointer = b.extractString(bufferPointer, name);

    unsigned int serverIDSize = serverID.size() + 1;
    unsigned int portSize = 2;
    unsigned int nameSize = name.size() + 1;
    unsigned int argTypesLength = (bufferSize - serverIDSize - portSize - nameSize) / 4;

    int argTypes[argTypesLength];
    b.extractArgTypes(bufferPointer, argTypes);

    addService(name, argTypes, serverID, port);
}

void handleLocRequest(Receiver &receiver, Sender &sender, char buffer[], unsigned int bufferSize) {
    string name;
    RWBuffer b;
    char * bufferPointer = buffer;
    bufferPointer = b.extractString(bufferPointer, name);

    unsigned int nameSize = name.size() + 1;

    unsigned int argTypesLength = (bufferSize - nameSize) / 4;
    int argTypes[argTypesLength];
    b.extractArgTypes(bufferPointer, argTypes);

    rpcFunctionKey key(name, argTypes);

    if (servicesDictionary.find(key) != servicesDictionary.end()) {
        cerr << "LOC REQ FOUND!" << endl;
        server_info *location = servicesDictionary[key]->front();
        cerr << "server_identifier = " << location->server_identifier << endl;
        cerr << "port = " << location->port << endl;
        sender.sendLocSuccessMessage(location->server_identifier, location->port);
    } else {
        cerr << "LOC REQ NOT FOUND!" << endl;
        sender.sendLocFailureMessage(FUNCTION_NOT_AVAILABLE);
    }
}

void handleRequest(int clientSocketFd, fd_set *master_set) {
    Receiver receiver(clientSocketFd);
    Sender sender(clientSocketFd);

    bool closed = false;
    unsigned int messageSize;
    if (chunkInfo[clientSocketFd] == 0) {
         //   cout << "nb" << nb << " " << numBytes<<endl;
        if(receiver.receiveMessageSize(messageSize) == 0 )
        {
            chunkInfo[clientSocketFd] = messageSize;

            MessageType msgType;

            if(receiver.receiveMessageType(msgType) == 0)
            {
                msgInfo[clientSocketFd] = msgType;
            }
            else
            {
                closed = true;
            }
            
        }
        else
        {
            closed = true;
        }
    } else {
        unsigned int size = chunkInfo[clientSocketFd];
        char buffer[size];
        if (receiver.receiveMessageGivenSize(size, buffer) == 0)
        {
            chunkInfo[clientSocketFd] = 0;
            MessageType msgType = msgInfo[clientSocketFd];

            if (msgType == REGISTER) {
                handleRegisterRequest(receiver, buffer, size);
            } else if (msgType == LOC_REQUEST) {
                handleLocRequest(receiver, sender, buffer, size);
            } else {
                cerr << "[BINDER] UNHANDLED MESSAGE TYPE: " << static_cast<int>(msgType) << endl;
            }
        }
        else
        {
            closed = true;
        }
    }

    if (closed) {
        chunkInfo[clientSocketFd] = 0;
        FD_CLR(clientSocketFd, master_set);
        close(clientSocketFd);
    }
}

int main(int argc, char *argv[]) {
    int localSocketFd = createSocket();
    if (localSocketFd < 0) {
        error("ERROR: Failed to open socket");
    }

    signal(SIGPIPE, SIG_IGN);
    listenOnSocket(localSocketFd);
    printSettings(localSocketFd);

    int max_fd = localSocketFd;
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    FD_SET(localSocketFd, &master_set);

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
                    handleRequest(clientSocketFd, &master_set);
                } else {
                    int newSocketFd = acceptConnection(localSocketFd);
                    max_fd = newSocketFd;
                    FD_SET(newSocketFd, &master_set);
                }
            }
        }
    }

    close(localSocketFd);

    return 0;
}