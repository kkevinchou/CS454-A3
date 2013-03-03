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

map<rpcFunctionKey, server_info> servicesDictionary;
map<int, unsigned int> chunkInfo;
map<int, MessageType> msgInfo;

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

    unsigned int argTypesLength = (bufferSize - serverID.size() - 1 - 2 - name.size() - 1)/4;

    int argTypes[argTypesLength];
    receiver.extractArgTypes(bufferPointer, argTypes);

    cerr << "serverID: "<<serverID<<endl;
    cerr << "port: "<< port<<endl;
    cerr << "name: "<<name<<endl;
    cerr << "argTypes: ";
    for(int i = 0; i < argTypesLength; i++)
    {
        cerr << argTypes[i] << " ";
    }
    cerr << endl;

    rpcFunctionKey key(name, argTypes);
    server_info location(serverID, port);
    servicesDictionary[key] = location;
}

void handleRequest(int clientSocketFd, fd_set *master_set) {
    Receiver receiver(clientSocketFd);
    bool closed = false;
    unsigned int messageSize;
    if (chunkInfo[clientSocketFd] == 0) {

         //   cout << "nb" << nb << " " << numBytes<<endl;
        if(receiver.receiveMessageSize(messageSize) == 0 )
        {
            chunkInfo[clientSocketFd] = messageSize;

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
                    cout << "accept connection"<<endl;
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