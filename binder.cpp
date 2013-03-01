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
#include "helpers.h"

using namespace std;

void printServerSettings(int localSocketFd) {
    char localHostName[256];
    gethostname(localHostName, 256);
    cout << "BINDER_ADDRESS " << localHostName << endl;

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(localSocketFd, (struct sockaddr *)&sin, &len);
    cout << "BINDER_PORT " << ntohs(sin.sin_port) << endl;
}

void toTitleCase(string &inStr) {
    bool capNext = true;

    for (unsigned int i = 0; i < inStr.length(); i++) {
        if (capNext && inStr[i] >= 97 && inStr[i] <= 122) {
            inStr[i] -= 32;
        }

        if (inStr[i] == 32) {
            capNext = true;
        } else {
            capNext = false;
        }
    }
}

string getStringFromBuffer(char buffer[], int n) {
    char charStr[n + 1];
    for (int i = 0; i < n; i++) {
        charStr[i] = buffer[i];
    }
    charStr[n] = '\0';

    string result = charStr;
    return result;
}

void handleRequest(int clientSocketFd, fd_set *master_set, map<int, unsigned int> &chunkInfo) {
    int bytesReceived;

    Receiver receiver(clientSocketFd);

    if (chunkInfo[clientSocketFd] == 0) {
        unsigned int numByes = receiver.receiveMessageSize();
        chunkInfo[clientSocketFd] = numBytes;
    } else {
        string recvStr = receiver.receiveMessageGivenSize(chunkInfo[clientSocketFd]);
        chunkInfo[clientSocketFd] = 0;
        string processedStr = recvStr;

        cout << recvStr << endl;
        toTitleCase(processedStr);

        Sender s(clientSocketFd);
        s.sendMessage(processedStr);

        delete buffer;
    }
}

int main(int argc, char *argv[])
{
    int localSocketFd = createSocket();
    if (localSocketFd < 0) {
        error("ERROR: Failed to open socket");
    }

    signal(SIGPIPE, SIG_IGN);
    listenOnSocket(localSocketFd);
    printServerSettings(localSocketFd);

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