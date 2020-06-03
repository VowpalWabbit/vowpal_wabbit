#include "test_generated.h"
#include <vector>
#include <fstream>
#include <iostream>

int main(){
    std::ifstream infile;
    infile.open("first.dat", std::ios::binary | std::ios::in);
    infile.seekg(0,std::ios::end);
    int length = infile.tellg();
    infile.seekg(0,std::ios::beg);
    char *buffer_pointer = new char[length];
    infile.read(buffer_pointer, length);

    auto data = GetData(buffer_pointer);

    std::cout << "Label of first example " << data->examples()->Get(0)->label() << std::endl;
}