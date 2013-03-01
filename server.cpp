#include <iostream>

#include "rpcserver.hpp"

using namespace std;

int sum (int a, int b)
{
	return a+b;
}
int sumSkeleton(int *, void **)
{

}

int main()
{
	rpcInit();



}