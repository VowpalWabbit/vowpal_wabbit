#include "test_generated.h"
#include "preamble.h"
#include <vector>
#include <fstream>
#include <iostream>

int main(){
    std::ifstream infile;
    infile.open("first.dat", std::ios::binary | std::ios::in);

    char raw_preamble[8];
    infile.read(raw_preamble, 8);
    VW::preamble p;
    p.read_from_bytes(reinterpret_cast<uint8_t*>(raw_preamble), 8);
    std::unique_ptr<char> msg_data(new char[p.msg_size]);

    auto data = GetData(msg_data.get());

    std::cout << "Label of first example " << data->examples()->Get(0)->label() << std::endl;
}