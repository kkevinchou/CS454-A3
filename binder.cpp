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

static map<rpcFunctionKey, list<server_info *> * > servicesDictionary;
static list<server_info *> roundRobinQueue;
static map<int, server_info *> fdToServerMap;
static map<int, unsigned int> chunkInfo;
static map<int, MessageType> msgInfo;

static bool terminating;
extern bool debug;

ReasonCode addService(string name, int argTypes[], string serverID, unsigned short port) {
    rpcFunctionKey key(name, argTypes);
    server_info location(serverID, port);

    ReasonCode r = SUCCESS;

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
            r = FUNCTION_OVERRIDDEN;
        }
    }

    hostList->push_back(l);
    return r;
}

void registerServer(string serverID, unsigned short port, int serverFd) {
    bool inQueue = false;
    server_info *location = new server_info(serverID, port);

    for (list<server_info *>::iterator it = roundRobinQueue.begin(); it != roundRobinQueue.end(); it++) {
        server_info *curLocation = (*it);
        if (*curLocation == *location) {
            inQueue = true;
        }
    }

    if (inQueue) {
        delete location;
    } else {
        roundRobinQueue.push_back(location);
        fdToServerMap[serverFd] = location;
    }
}

void removeServer(int serverFd) {
    if (fdToServerMap.find(serverFd) == fdToServerMap.end()) {
        return;
    }

    server_info *server = fdToServerMap[serverFd];

    // cerr << "roundRobinQueue before size = " << roundRobinQueue.size() << endl;
    roundRobinQueue.remove(server);
    // cerr << "roundRobinQueue after size = " << roundRobinQueue.size() << endl;

    // cerr << "fdToServerMap before size = " << fdToServerMap.size() << endl;
    map<int, server_info *>::iterator it = fdToServerMap.find(serverFd);
    if (it != fdToServerMap.end()) {
        fdToServerMap.erase(it);
    }
    // cerr << "fdToServerMap after size = " << fdToServerMap.size() << endl;

    for (map<rpcFunctionKey, list<server_info *> * >::iterator it = servicesDictionary.begin(); it != servicesDictionary.end(); it++) {
        list<server_info *> *hostList = it->second;
        for (list<server_info *>::iterator it2 = hostList->begin(); it2 != hostList->end(); it2++) {
            if (*server == *(*it2)) {
                // cerr << "hostList before size = " << hostList->size() << endl;
                hostList->erase(it2);
                // cerr << "hostList after size = " << hostList->size() << endl;
                break;
            }
        }
        // TODO : Clean up the memory for the key when the host list is empty?
    }

    delete server;

    chunkInfo[serverFd] = 0;
    msgInfo[serverFd] = ERROR;
}

void handleRegisterRequest(Receiver &receiver, Sender &sender, char buffer[], unsigned int bufferSize, int serverFd) {
    bool failed = false;
    ReasonCode r = SUCCESS;

    try {
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

        registerServer(serverID, port, serverFd);
        r = addService(name, argTypes, serverID, port);
    } catch (int e) {
        failed = true;
        r = FAILURE;
        removeServer(serverFd);
    }

    if (!failed) {
        sender.sendRegisterSuccessMessage(r);
    } else {
        sender.sendRegisterFailureMessage(r);
    }
}

server_info *getRoundRobinServer(const rpcFunctionKey &key) {
    if (servicesDictionary.find(key) == servicesDictionary.end()) {
        return NULL;
    }

    list<server_info *> *availServers = servicesDictionary[key];

    server_info *selectedServer = NULL;
    for (list<server_info *>::iterator it = roundRobinQueue.begin(); it != roundRobinQueue.end(); it++) {
        server_info *nextRRServer = (*it);
        for (list<server_info *>::iterator it2 = availServers->begin(); it2 != availServers->end(); it2++) {
            server_info *supportingServer = (*it2);
            if (*nextRRServer == *supportingServer) {
                selectedServer = nextRRServer;
            }
        }
        if (nextRRServer != NULL) {
            // cerr << "front port was: " << roundRobinQueue.front()->port << endl;
            roundRobinQueue.splice(roundRobinQueue.end(), roundRobinQueue, it);
            // cerr << "front port is: " << roundRobinQueue.front()->port << endl;
            // cerr << "last port is: " << roundRobinQueue.back()->port << endl;
        }
    }

    return selectedServer;
}

void handleLocRequest(Receiver &receiver, Sender &sender, char buffer[], unsigned int bufferSize) {
    try {
        string name;
        RWBuffer b;
        char * bufferPointer = buffer;
        bufferPointer = b.extractString(bufferPointer, name);

        unsigned int nameSize = name.size() + 1;

        unsigned int argTypesLength = (bufferSize - nameSize) / 4;
        int argTypes[argTypesLength];
        b.extractArgTypes(bufferPointer, argTypes);

        rpcFunctionKey key(name, argTypes);

        server_info *location = getRoundRobinServer(key);

        if (location != NULL) {
            cerr << "LOC REQ FOUND!" << endl;
            cerr << "server_identifier = " << location->server_identifier << endl;
            cerr << "port = " << location->port << endl;
            sender.sendLocSuccessMessage(location->server_identifier, location->port);
        } else {
            cerr << "LOC REQ NOT FOUND!" << endl;
            sender.sendLocFailureMessage(FUNCTION_NOT_AVAILABLE);
        }
    } catch (int e) {
        sender.sendLocFailureMessage(FAILURE);
    }
}

void handleTerminateRequest() {
    terminating = true;

    for (map<int, server_info *>::iterator it = fdToServerMap.begin(); it != fdToServerMap.end(); it++) {
        int serverFd = it->first;
        Sender s(serverFd);
        s.sendTerminateMessage();
    }

    list<int> removeList;
    while (fdToServerMap.size() > 0) {
        cerr << "Waiting on " << fdToServerMap.size() << " server(s) to terminate" << endl;
        for (map<int, server_info *>::iterator it = fdToServerMap.begin(); it != fdToServerMap.end(); it++) {
            int serverFd = it->first;
            server_info *server = it->second;

            int socketFd = setupSocketAndReturnDescriptor(server->server_identifier.c_str(), server->port);

            if (socketFd < 0) {
                removeList.push_back(serverFd);
            }
        }

        for (list<int>::iterator it = removeList.begin(); it != removeList.end(); it++) {
            removeServer(*it);
        }
        removeList.clear();

        sleep(1);
    }

    cerr << "All servers have terminated" << endl;

    // TODO: clean up memory
}

void handleRequest(int clientSocketFd, fd_set *master_set) {
    Receiver receiver(clientSocketFd);
    Sender sender(clientSocketFd);

    bool closed = false;
    unsigned int messageSize;
    if (chunkInfo[clientSocketFd] == 0) {
        if(receiver.receiveMessageSize(messageSize) == 0 )
        {
            chunkInfo[clientSocketFd] = messageSize;

            MessageType msgType;

            if(receiver.receiveMessageType(msgType) == 0)
            {
                msgInfo[clientSocketFd] = msgType;
                if (msgType == TERMINATE) {
                    handleTerminateRequest();
                }
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
                handleRegisterRequest(receiver, sender, buffer, size, clientSocketFd);
            } else if (msgType == LOC_REQUEST) {
                handleLocRequest(receiver, sender, buffer, size);
            } else if (msgType == TERMINATE) {
                handleTerminateRequest();
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
        cerr << "[BINDER] Socket detected to be closed!" << endl;
        chunkInfo[clientSocketFd] = 0;
        removeServer(clientSocketFd);
        FD_CLR(clientSocketFd, master_set);
        close(clientSocketFd);
    }
}

int main(int argc, char *argv[]) {
    int localSocketFd = createSocket();
    if (localSocketFd < 0) {
        cerr << "ERROR: Failed to open socket" << endl;
        return SOCKET_OPEN_FAILURE;
    }

    signal(SIGPIPE, SIG_IGN);
    listenOnSocket(localSocketFd);
    printSettings(localSocketFd);

    int max_fd = localSocketFd;
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    FD_SET(localSocketFd, &master_set);

    terminating = false;
    while (!terminating) {
        memcpy(&working_set, &master_set, sizeof(master_set));
        int selectResult = select(max_fd + 1, &working_set, NULL, NULL, NULL);

        if (selectResult < 0) {
            cerr << "ERROR: Select failed" << endl;
            return SELECT_FAILED;
        } else if (selectResult == 0) {
            cerr<<"ERROR: Select timed out"<< endl;
            return SELECT_TIMED_OUT;
        }

        for (int i = 0; i < max_fd + 1; i++) {
            if (FD_ISSET(i, &working_set)) {
                if (i != localSocketFd) {
                    int clientSocketFd = i;
                    handleRequest(clientSocketFd, &master_set);

                    if (terminating) {
                        break;
                    }
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