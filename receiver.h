#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>
#include "constants.h"

using namespace std;

class Receiver
{
	private:
		int _sfd;
		short convertToShort(char d[2]);
		long convertToLong(char d[4]);
		float convertToFloat(char d[4]);
		double convertToDouble(char d[8]);
		unsigned int convertToUnsignedInt(char d[4]);
		int convertToInt(char d[4]);



	public:
		Receiver(int socketFileDescriptor);
		string receiveMessageAsString(); // THIS BLOCKS UNTIL A FULL MESSAGE IS RECEIVED. Returns NULL on failure
		int receiveMessageGivenSize(unsigned int messageSize, char ret[]); // returns a message char * of length MessageSize
		int receiveMessageSize(unsigned int &i); // returns a positive int (4 bytes)
		
		MessageType receiveMessageType();

		int receiveRegisterMessage(string &serverID, int &port, string &name, int argTypes[]);


		
		char * extractString(char * head, string &s);
		char * extractArgTypes(char * head, int argTypes[]); // make sure enough memory is allocated in argTypes

		char * extractIntArray(char * head, int intArray[], unsigned int length); // make sure enough memory is in intArray
		char * extractShortArray(char * head, short shortArray[], unsigned int length); 
		char * extractCharArray(char * head, char charArray[], unsigned int length); 
		char * extractLongArray(char * head, long longArray[], unsigned int length); 
		char * extractDoubleArray(char * head, double doubleArray[], unsigned int length); 
		char * extractFloatrray(char * head, float floatArray[], unsigned int length); 


		char * extractUnsignedInt(char * head, unsigned int &i);
		char * extractInt(char * head, int &i);
		char * extractShort(char * head, short &i);
		char * extractLong(char * head, long &l);
		char * extractFloat(char * head, float &f);
		char * extractDouble(char * head, double &d);

		unsigned int returnArgTypesLength(int * head);

		unsigned short getArrayLengthFromArgumentType(int argType);
		int getTypeFromArgumentType(int argType);
};
#endif