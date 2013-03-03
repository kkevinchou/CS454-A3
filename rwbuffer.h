#ifndef RWBUFFER_H
#define RWBUFFER_H


#include "constants.h"

class RWBuffer
{
	private:
		DataTypeConversion _convertor;
		char * copyBufferToConvertor(char * bufferP, unsigned int length);
		char * copyConvertorToBuffer(char * bufferP, unsigned int length);
	public:
		// every function that returns char * returns the new position of the buffer pointer


		// insertion methods, adds data from the provided var to the buffer
		char *insertShortToBuffer(short s, char *bufferP);
        char *insertIntToBuffer(int i, char *bufferP);
        char *insertStringToBuffer(string s, char *bufferP);
        char *insertUnsignedIntToBuffer(unsigned int u, char *bufferP);
        char *insertCharToBuffer(char c, char *bufferP);
        char *insertDoubleToBuffer(double d, char *bufferP);
        char *insertFloatToBuffer(float f, char *bufferP);
        char *insertLongToBuffer(long l, char *bufferP);
        // insert arrays of data
		char *insertCStringToBuffer(char * c, char *bufferP );
     	char *insertIntArrayToBuffer(int intBuf[], int numInts, char *bufferP);
		char * insertDoubleArrayToBuffer(double doubleArray[], int length, char *bufferP);
		char * insertFloatArrayToBuffer(float floatArray[], int length, char *bufferP) ;
		char * insertCharArrayToBuffer(char charArray[], int length, char *bufferP) ;
		char * insertShortArrayToBuffer(short shortArray[], int length, char *bufferP);
		char * insertLongArrayToBuffer(long longArray[], int length, char *bufferP);

		// extraction methods, removes data from the buffer to an out var
		char * extractString(char * bufferP, string &s); // use this to extract cstrings
		// treats bufferP as int*, extracts until a 0 is found and include the 0 too
		char * extractArgTypes(char * bufferP, int argTypes[]); // make sure enough memory is allocated in argTypes
		// extract arrays of data
		char * extractIntArray(char * bufferP, int intArray[], unsigned int length); // make sure enough memory is in intArray
		char * extractShortArray(char * bufferP, short shortArray[], unsigned int length); 
		char * extractCharArray(char * bufferP, char charArray[], unsigned int length); 
		char * extractLongArray(char * bufferP, long longArray[], unsigned int length); 
		char * extractDoubleArray(char * bufferP, double doubleArray[], unsigned int length); 
		char * extractFloatArray(char * bufferP, float floatArray[], unsigned int length); 
		// extract different data types
		char * extractUnsignedInt(char * bufferP, unsigned int &i);
		char * extractInt(char * bufferP, int &i);
		char * extractShort(char * bufferP, short &i);
		char * extractLong(char * bufferP, long &l);
		char * extractFloat(char * bufferP, float &f);
		char * extractDouble(char * bufferP, double &d);
		char * extractChar(char * bufferP, char &c);

		// helpers:
		// treats bufferP as an int*, iterates until a 0 integer is found, returns length including 0
		unsigned int returnArgTypesLength(char * bufferP);
		// gets the third and fourth bytes, and translates it into a short
		unsigned short getArrayLengthFromArgumentType(int argType);
		// gets the second byte and translate it into an int
		int getTypeFromArgumentType(int argType);
};



#endif