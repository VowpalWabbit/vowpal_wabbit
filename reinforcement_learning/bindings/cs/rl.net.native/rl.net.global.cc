#include "rl.net.global.h"

#include <iostream>

API void SayHello(void)
{
    std::cout << "Hello World!" << std::endl;
}

API void Delete(void* pointer)
{
    delete pointer;
}

API void DeleteArray(void* pointer)
{
    delete [] pointer;
}
