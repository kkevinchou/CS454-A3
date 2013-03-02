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

struct location {
    string id;
    int port;
};

void printSettings(int localSocketFd) {
    char localHostName[256];
    gethostname(localHostName, 256);
    cout << "BINDER_ADDRESS " << localHostName << endl;

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(localSocketFd, (struct sockaddr *)&sin, &len);
    cout << "BINDER_PORT " << ntohs(sin.sin_port) << endl;
}

void handleRequest(int clientSocketFd, fd_set *master_set, map<int, unsigned int> &chunkInfo) {
    cout << "handle"<<endl;
    Receiver receiver(clientSocketFd);

    if (chunkInfo[clientSocketFd] == 0) {
        int numBytes = receiver.receiveMessageSize();

         //   cout << "nb" << nb << " " << numBytes<<endl;
        if(numBytes >= 0 )
        {
            unsigned int nb = (unsigned int)numBytes;
            chunkInfo[clientSocketFd] = nb;
            if(receiver.receiveMessageType() == REGISTER) cout << "Received REGISTER message"<<endl;
        }
        else
        {
            //failed
            chunkInfo[clientSocketFd] = 0;
        }
    } else {
        unsigned int size = chunkInfo[clientSocketFd];
        char buffer[size];
        if(receiver.receiveMessageGivenSize(size, buffer) == 0)
        {
            chunkInfo[clientSocketFd] = 0;
            string recvStr(buffer, buffer+size);
            cout << recvStr << endl;

            Sender s(clientSocketFd);
            s.sendMessage(recvStr);
        }
        else
        {
            //failed
            chunkInfo[clientSocketFd] = 0;
        }


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
                    int n = 0;
                    ioctl(i, FIONREAD, &n);
              
                    if(n > 0)
                    {
                        int clientSocketFd = i;
                        handleRequest(clientSocketFd, &master_set, chunkInfo);
                    }
                        
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