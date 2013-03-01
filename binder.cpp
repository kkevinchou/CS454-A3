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

    if (chunkInfo[clientSocketFd] == 0) {
        unsigned char *buffer = new unsigned char[4];
        memset(buffer, 0, 4);
        bytesReceived = recv(clientSocketFd, buffer, 4, 0);

        unsigned int numBytes = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
        chunkInfo[clientSocketFd] = numBytes;

        if (bytesReceived < 0) {
            error("ERROR: Failed to read from socket");
        }

        if (bytesReceived == 0) {
            FD_CLR(clientSocketFd, master_set);
            close(clientSocketFd);
            delete buffer;
            return;
        }

        delete buffer;
    } else {
        int bufferSize = 256;
        char *buffer = new char[bufferSize];
        string recvStr;

        while (chunkInfo[clientSocketFd] > 0) {
            bytesReceived = recv(clientSocketFd, buffer, bufferSize, 0);
            chunkInfo[clientSocketFd] -= bytesReceived;

            if (bytesReceived < 0) {
                error("ERROR: Failed to read from socket");
            }

            if (bytesReceived == 0) {
                FD_CLR(clientSocketFd, master_set);
                close(clientSocketFd);
                return;
            }

            string stringChunk = getStringFromBuffer(buffer, bytesReceived);
            recvStr += stringChunk;
        }

        string processedStr = recvStr;

        cout << recvStr << endl;
        toTitleCase(processedStr);

        //
        Sender s(clientSocketFd);
        s.sendMessage(processedStr);
        //
        // unsigned int size = processedStr.length() + 1;

        // unsigned char sizeBytes[4];
        // unsigned char *sizeBytesP = sizeBytes;
        // const char *cstr = processedStr.c_str();

        // sizeBytes[0] = (size >> 24) & 0xFF;
        // sizeBytes[1] = (size >> 16) & 0xFF;
        // sizeBytes[2] = (size >> 8) & 0xFF;
        // sizeBytes[3] = size & 0xFF;

        // if (send(clientSocketFd, sizeBytesP, 4, 0) < 0) {
        //     error("ERROR: Failed sending to socket1");
        // }

        // unsigned int bytesLeftToSend = size;

        // while(true) {
        //     int bytesSent = send(clientSocketFd, cstr, bytesLeftToSend, 0);

        //     if(bytesSent == 0) {
        //         break;
        //     }
        //     else if (bytesSent < 0) {
        //         error("ERROR: Failed sending to socket2");
        //     }

        //     bytesLeftToSend -= bytesSent;
        //     cstr += bytesSent;
        // }

        delete buffer;
    }
}

int main(int argc, char *argv[])
{
    int localSocketFd = socket(AF_INET, SOCK_STREAM, 0);
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