#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

#include "rpcserver.h" // for the typedef

using namespace std;

static map<rpcFunctionKey, server_info> servicesDictionary;
static map<int, unsigned int> chunkInfo;
static map<int, MessageType> msgInfo;

void printSettings(int localSocketFd) {
    char localHostName[256];
    gethostname(localHostName, 256);
    cout << "BINDER_ADDRESS " << localHostName << endl;

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(localSocketFd, (struct sockaddr *)&sin, &len);
    cout << "BINDER_PORT " << ntohs(sin.sin_port) << endl;
}

extern bool debug;

void handleRegisterRequest(Receiver &receiver, char buffer[], unsigned int bufferSize) {
    string serverID;
    short port;
    string name;

    char * bufferPointer = buffer;
    bufferPointer = receiver.extractString(bufferPointer, serverID);
    bufferPointer = receiver.extractShort(bufferPointer, port);
    bufferPointer = receiver.extractString(bufferPointer, name);

    unsigned int serverIDSize = serverID.size() + 1;
    unsigned int portSize = 2;
    unsigned int nameSize = name.size() + 1;

    unsigned int argTypesLength = (bufferSize - serverIDSize - portSize - nameSize) / 4;
    int *argTypes = (int *)malloc(argTypesLength * sizeof(int));
    receiver.extractArgTypes(bufferPointer, argTypes);

    rpcFunctionKey key(name, argTypes);
    server_info location(serverID, port);

    if (servicesDictionary.find(key) != servicesDictionary.end()) {
        cerr << "Registration of given function found, ignoring registration!" << endl;
        free(argTypes);
    } else {
        cerr << "Registration of given function not found, registering now..." << endl;
        servicesDictionary[key] = location;
    }
}

void handleLocRequest(Receiver &receiver, char buffer[], unsigned int bufferSize) {
    string name;

    char * bufferPointer = buffer;
    bufferPointer = receiver.extractString(bufferPointer, name);

    unsigned int nameSize = name.size() + 1;

    unsigned int argTypesLength = (bufferSize - nameSize) / 4;
    int argTypes[argTypesLength];
    receiver.extractArgTypes(bufferPointer, argTypes);

    rpcFunctionKey key(name, argTypes);

    if (servicesDictionary.find(key) != servicesDictionary.end()) {
        cerr << "LOC REQ FOUND!" << endl;
    } else {
        cerr << "LOC REQ NOT FOUND!" << endl;
    }
}

void handleRequest(int clientSocketFd, fd_set *master_set) {
    Receiver receiver(clientSocketFd);
    bool closed = false;

    if (chunkInfo[clientSocketFd] == 0) {
        int numBytes = receiver.receiveMessageSize();
        if(numBytes >= 0 )
        {
            unsigned int nb = (unsigned int)numBytes;
            chunkInfo[clientSocketFd] = nb;

            MessageType msgType = receiver.receiveMessageType();
            msgInfo[clientSocketFd] = msgType;
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
            switch (msgInfo[clientSocketFd]) {
                case REGISTER:
                {
                    handleRegisterRequest(receiver, buffer, size);
                    break;
                }
                case LOC_REQUEST:
                {
                    handleLocRequest(receiver, buffer, size);
                    break;
                }
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