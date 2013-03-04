#include "rwbuffer.h"
#include <iostream>

using namespace std;

// byte helpers
unsigned short RWBuffer::getArrayLengthFromArgumentType(int argType)
{
    return argType & 65534;
}
int RWBuffer::getTypeFromArgumentType(int argType)
{
    return (argType >> 16) & 255;
}
unsigned int RWBuffer::returnArgTypesLength(int * array)
{
	// keeps incrementing pointer until we find a 0
    unsigned int counter = 0;
    while(true)
    {
        counter++;

        int type = *array;
        array++;
        if(type == 0) break;

    }

    return counter;
}

unsigned int RWBuffer::returnArgTypesLength(char * bufferP)
{
    // keeps incrementing pointer until we find a 0
    unsigned int counter = 0;
    while(true)
    {
        counter++;

        int type;
        bufferP = extractInt(bufferP, type);
        if(type == 0) break;

    }

    return counter;
}

char * RWBuffer::copyBufferToConvertor(char * bufferP, unsigned int length)
{
	if(length > 8) cerr << "WARNING: union DataTypeConvertor may not work correctly."<<endl;

    for(unsigned int i = 0; i < length; i++)
    {
        _convertor.charArray[i] = *bufferP;
        bufferP++;
    }
    return bufferP;
}
// extract arrays
char * RWBuffer::extractIntArray(char * bufferP, int intArray[], unsigned int length) // make sure enough memory is in intArray
{
    for(unsigned int i = 0; i < length; i++)
    {
        int n;
        bufferP = extractInt(bufferP, n);
        intArray[i] = n;
    }
    return bufferP;
}
char * RWBuffer::extractShortArray(char * bufferP, short shortArray[], unsigned int length)
{
    for(unsigned int i = 0; i < length; i++)
    {
        short n;
        bufferP = extractShort(bufferP, n);
        shortArray[i] = n;
    }
    return bufferP;
}
char * RWBuffer::extractCharArray(char * bufferP, char charArray[], unsigned int length)
{
    for(unsigned int i = 0; i < length; i++)
    {
        char n;
        bufferP = extractChar(bufferP, n);
        charArray[i] = n;
    }
    return bufferP;
}
char * RWBuffer::extractLongArray(char * bufferP, long longArray[], unsigned int length)
{
    for(unsigned int i = 0; i < length; i++)
    {
        long n;
        bufferP = extractLong(bufferP, n);
        longArray[i] = n;
    }
    return bufferP;
}
char * RWBuffer::extractDoubleArray(char * bufferP, double doubleArray[], unsigned int length)
{
    for(unsigned int i = 0; i < length; i++)
    {
        double n;
        bufferP = extractDouble(bufferP, n);
        doubleArray[i] = n;
    }
    return bufferP;
}
char * RWBuffer::extractFloatArray(char * bufferP, float floatArray[], unsigned int length)
{
    for(unsigned int i = 0; i < length; i++)
    {
        float n;
        bufferP = extractFloat(bufferP, n);
        floatArray[i] = n;
    }
    return bufferP;
}
// make sure enough memory is allocated in argTypes. Treats char * as int *
// and stops on a 0
char * RWBuffer::extractArgTypes(char * bufferP, int argTypes[])
{
    int index = 0;
    while(true)
    {
        int i;
        bufferP = extractInt(bufferP, i);
        argTypes[index] = i;
        index++;
        if(i ==0) break;

    }
    return bufferP;
}

// Extract Types
char * RWBuffer::extractLong(char * bufferP, long &l)
{
    bufferP = copyBufferToConvertor(bufferP, sizeof(long));
    l = _convertor.l;
    return bufferP;
}
char * RWBuffer::extractFloat(char * bufferP, float &f)
{
    bufferP = copyBufferToConvertor(bufferP, sizeof(float));
    f = _convertor.f;
    return bufferP;
}
char * RWBuffer::extractChar(char * bufferP, char &c)
{
    c = *bufferP;
    bufferP++;
    return bufferP;
}
char * RWBuffer::extractDouble(char * bufferP, double &d)
{
    bufferP = copyBufferToConvertor(bufferP, sizeof(double));
    d = _convertor.d;
    return bufferP;
}
char * RWBuffer::extractUnsignedInt(char * bufferP, unsigned int &i)
{
    bufferP = copyBufferToConvertor(bufferP, sizeof(unsigned int));
    i= _convertor.ui;
    return bufferP;
}
char * RWBuffer::extractInt(char * bufferP, int &i)
{
    bufferP = copyBufferToConvertor(bufferP, sizeof(int));
    i = _convertor.i;
    return bufferP;
}

// extracts until '\0', you can use this to extract cstrings
char * RWBuffer::extractString(char * bufferP, string &s)
{
    string r = "";
    while(*bufferP != '\0')
    {
        //cout << "extracting "<< *bufferP<<endl;
        r += *bufferP;
        bufferP++;
    }
    s = r;
    bufferP++;
    return bufferP;
}

char * RWBuffer::extractShort(char * bufferP, short &i)
{
    bufferP = copyBufferToConvertor(bufferP, sizeof(short));
    i = _convertor.s;
    return bufferP;
}

char * RWBuffer::extractUnsignedShort(char * bufferP, unsigned short &us)
{
    bufferP = copyBufferToConvertor(bufferP, sizeof(unsigned short));
    us = _convertor.us;
    return bufferP;
}

// insertion methods
char *RWBuffer::insertCStringToBuffer(const char * c, char *bufferP )
{
	while(*c != '\0')
	{
		*bufferP = *c;
		bufferP++;
		c++;
	}
	*bufferP = *c; // insert the '\0' as well
	bufferP++;
	return bufferP;
}
char * RWBuffer::insertStringToBuffer(string s, char *bufferP)
{
	unsigned int i = 0;
	for (i = 0; i < s.size(); i++) {
		bufferP[i] = s[i];
	}
	bufferP[i] = '\0';

	return bufferP + i + 1;
}

char * RWBuffer::copyConvertorToBuffer(char * bufferP, unsigned int length)
{
	for(unsigned int index = 0; index < length; index++)
	{
		bufferP[index] = _convertor.charArray[index];
	}
	return bufferP+length;
}
char * RWBuffer::insertIntToBuffer(int i, char *bufferP)
{
	_convertor.i = i;
	return copyConvertorToBuffer(bufferP, sizeof(int));
}

char * RWBuffer::insertUnsignedIntToBuffer(unsigned int u, char *bufferP)
{
	_convertor.ui = u;
	return copyConvertorToBuffer(bufferP, sizeof(unsigned int));
}

char * RWBuffer::insertShortToBuffer(short s, char *bufferP)
{
	_convertor.s = s;
	return copyConvertorToBuffer(bufferP, sizeof(short));
}

char * RWBuffer::insertIntArrayToBuffer(int intArray[], int length, char *bufferP) {
	for (int i = 0; i < length; i++) {
		bufferP = insertIntToBuffer(intArray[i], bufferP);
	}

	return bufferP;
}

char * RWBuffer::insertDoubleArrayToBuffer(double doubleArray[], int length, char *bufferP) {
	for (int i = 0; i < length; i++) {
		bufferP = insertDoubleToBuffer(doubleArray[i], bufferP);
	}

	return bufferP;
}


char * RWBuffer::insertFloatArrayToBuffer(float floatArray[], int length, char *bufferP) {
	for (int i = 0; i < length; i++) {
		bufferP = insertFloatToBuffer(floatArray[i], bufferP);
	}

	return bufferP;
}

char * RWBuffer::insertShortArrayToBuffer(short shortArray[], int length, char *bufferP) {
	for (int i = 0; i < length; i++) {
		bufferP = insertShortToBuffer(shortArray[i], bufferP);
	}

	return bufferP;
}
char * RWBuffer::insertCharArrayToBuffer(char charArray[], int length, char *bufferP)
{
	for (int i = 0; i < length; i++) {
		bufferP = insertCharToBuffer(charArray[i], bufferP);
	}

	return bufferP;
}
char * RWBuffer::insertLongArrayToBuffer(long longArray[], int length, char *bufferP) {
	for (int i = 0; i < length; i++) {
		bufferP = insertLongToBuffer(longArray[i], bufferP);
	}

	return bufferP;
}
char *RWBuffer::insertCharToBuffer(char c, char *bufferP) {
	bufferP[0] = c;
	return bufferP + 1;
}

char *RWBuffer::insertDoubleToBuffer(double d, char *bufferP) {
 	_convertor.d = d;
	return copyConvertorToBuffer(bufferP, sizeof(double));
}

 char *RWBuffer::insertFloatToBuffer(float f, char *bufferP) {
	_convertor.f = f;
	return copyConvertorToBuffer(bufferP, sizeof(float));
 }

char *RWBuffer::insertLongToBuffer(long l, char *bufferP) {
	_convertor.l = l;
	return copyConvertorToBuffer(bufferP, sizeof(long));
}
char *RWBuffer::insertUnsignedShortToBuffer(unsigned short us, char *bufferP)
{
    _convertor.us = us;
    return copyConvertorToBuffer(bufferP, sizeof(unsigned short));
}

