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

    SUCCESS = 0,
    FAILURE = -1,
    FUNCTION_OVERRIDDEN = 2,
    FUNCTION_NOT_AVAILABLE = -2,
    SOCKET_SEND_FAILURE = -3,
    SOCKET_OPEN_FAILURE = -4,
    SOCKET_UNKNOWN_HOST = -5,
    SOCKET_CONNECTION_FALIURE = -6,
    SOCKET_LOCAL_BIND_FALIURE = -7,
    SOCKET_ACCEPT_CLIENT_FAILURE = -8,
    SOCKET_RECEIVE_FAILURE = -9,
    EXECUTE_UNKNOWN_SKELETON = -10,
    INIT_UNSET_BINDER_ADDRESS = -11,
    INIT_UNSET_BINDER_PORT = -12,
    INIT_BINDER_SOCKET_FAILURE = -13,
    INIT_LOCAL_SOCKET_FAILURE = -14,
    RECEIVE_INVALID_MESSAGE_TYPE = -15,
    SELECT_FAILED = -16,
    SELECT_TIMED_OUT = -17











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

struct service_info {
    string server_identifier;
    unsigned short port;
    struct rpcFunctionKey *functionKey;

    service_info(string server_identifier, unsigned short port, struct rpcFunctionKey *functionKey) : server_identifier(server_identifier), port(port), functionKey(functionKey) {}
    service_info() {
        server_identifier = "";
        port = 0;
        functionKey = NULL;
    }

    service_info(const struct service_info &r) {
        server_identifier = r.server_identifier;
        port = r.port;
        functionKey = r.functionKey;
    }
};

bool operator == (const server_info &l, const service_info &r);
bool operator == (const service_info &l, const service_info &r);
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