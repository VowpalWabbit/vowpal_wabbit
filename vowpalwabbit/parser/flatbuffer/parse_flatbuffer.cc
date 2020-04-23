#include "parse_flatbuffer.h"
#include "test_generated.h"

namespace VW
{

void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples)
{
    std::cout << "Hello0";
}

int flatbuffer_to_examples(vw* all, v_array<example*>& examples)
{
    // Should shift reading to a separate function
    std::ifstream infile;
    infile.open(all->data_filename, std::ios::binary | std::ios::in);
    infile.seekg(0,std::ios::end);
    int length = infile.tellg();
    infile.seekg(0,std::ios::beg);
    char *buffer_pointer = new char[length];
    infile.read(buffer_pointer, length);

    auto data = GetData(buffer_pointer);
    
    example* ae = examples[0];
    all->p->lp = 
    all->p->lp.default_label() 

}
} // VW