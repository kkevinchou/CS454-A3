#include "constants.h"
#include <cstring>

#include <iostream>

using namespace std;

// bool operator == (const rpcFunctionKey &l, const rpcFunctionKey &r)
// {
//     if(l.name == r.name)
//     {
//         int i = 0;
//         int lArgs = l.argTypes[i];
//         int rArgs = r.argTypes[i];
//         // structs with less args are smaller
//         // structs with the same args are the same
//         // structs with the same number args,
//         // but different arg values,
//         // the one with the smaller value is smaller
//         while(lArgs!= 0 && rArgs != 0)
//         {
//             if(lArgs != rArgs)
//             {
//                 return false;
//             }
//             i++;
//             lArgs = l.argTypes[i];
//             rArgs = r.argTypes[i];
//         }

//         if(rArgs == 0 && lArgs == 0)
//         {
//             // have same number of args with same type
//             return true;
//         }
//         else
//         {
//             // have different number of args
//             return false;
//         }
//     }
//     else
//     {
//         return false;
//     }
// }

bool operator < (const rpcFunctionKey &l, const rpcFunctionKey &r)
{
    // cerr << "operator <!" << endl;
    if(l.name == r.name)
    {
        int i = 0;
        int lArgs = l.argTypes[i];
        int rArgs = r.argTypes[i];

        // cerr << "lArgs = " << lArgs << endl;
        // cerr << "rArgs = " << rArgs << endl;

        // structs with less args are smaller
        // structs with the same args are the same
        // structs with the same number args,
        // but different arg values,
        // the one with the smaller value is smaller
        while(lArgs!= 0 && rArgs != 0)
        {
            if(lArgs != rArgs)
            {
                // cerr << "DIFF : " << "lArgs = " << lArgs << ", " << "rArgs = " << rArgs << endl;
                return lArgs < rArgs;
            }
            i++;
            lArgs = l.argTypes[i];
            rArgs = r.argTypes[i];
        }

        if(rArgs == 0) // || lArgs == 0
        {
            // l is longer, so l > r
            // or same length with same parameters, so l == r, which means l < r is false
            return false;
        }
        else
        {
            // r is longer, so l < r
            return true;
        }
    }
    else
    {
        // c > 0 if l > r
        // c < 0 if l < r
        return (l.name < r.name);
    }
}