#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

using namespace std;

union DataTypeConversion
{
    unsigned int ui;
    int i;
    double d;
    float f;
    long l;
    short s;
    unsigned short us;
    char charArray[8];
};

enum MessageType
{
	ERROR = 0,
	REGISTER = 1,
	LOC_REQUEST = 2,
	LOC_SUCCESS = 3,
	LOC_FAILURE = 4,
	EXECUTE = 5,
	EXECUTE_SUCCESS = 6,
	EXECUTE_FAILURE = 7,
    REGISTER_SUCCESS = 8,
    REGISTER_FAILURE = 9,
	TERMINATE = 10
};

enum ReasonCode
{
    FUNCTION_NOT_AVAILABLE = -2,
    FUNCTION_OVERRIDE = 2,
    SUCCESS = 0
};

struct server_info {
    string server_identifier;
    unsigned short port;
    server_info(string server_identifier, unsigned short port) : server_identifier(server_identifier), port(port) {}
    server_info() {
        server_identifier = "";
        port = 0;
    }

    server_info(const struct server_info &r) {
        server_identifier = r.server_identifier;
        port = r.port;
    }
};

bool operator == (const server_info &l, const server_info &r);

struct rpcFunctionKey
{
    string name;
    int *argTypes;
    rpcFunctionKey(string name, int *argTypes) : name(name), argTypes(argTypes){}

    rpcFunctionKey(const struct rpcFunctionKey & r) {
        name = r.name;
        argTypes = r.argTypes;
    }
};

bool operator < (const rpcFunctionKey &l, const rpcFunctionKey &r);

#endif