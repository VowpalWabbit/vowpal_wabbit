#include <fstream>
#include <iostream>

#include "parse_flatbuffer.h"
#include "test_generated.h"
#include "global_data.h"
#include "example.h"

namespace VW
{
void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples)
{
  std::cout << "Hello0\n";
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


    // We have the data from flatbuffer now.
    auto data = GetData(buffer_pointer);

    // std::cout << "Label of first example " << data->examples()->Get(0)->label() << std::endl;

    // Parse examples

    parse_flatbuf_examples(all, examples, data);
}

void parse_flatbuf_examples(vw* all, v_array<example*>& examples, const Data* data)
{
    for (int index=0; index<data->examples()->size(); index++){
        // Get unused example
        examples.push_back(&VW::get_unused_example(all));
        // Parse individual example
        parse_flatbuf_example(all, examples[index], data->examples()->Get(index), index);
    }
}

void parse_flatbuf_example(vw* all, example* ae, const Datapoint* datapoint, int index)
{
    all->p->lp.default_label(&ae->l);

    unsigned char _index = (unsigned char)' ';
    bool _new_index = false;
    if (ae->feature_space[_index].size() == 0)
        bool _new_index = true;

    features& fs = ae->feature_space[_index];

    for (int j=0; j<datapoint->features()->size(); j++)
    {
        parse_flatbuf_features(all, ae, fs, datapoint->features()->Get(j));
    }

    if (_new_index && ae->feature_space[_index].size() > 0)
      ae->indices.push_back(_index);

}

void parse_flatbuf_features(vw* all, example* ae, features& fs, const Feature* feature)
{
    uint64_t word_hash;
    // Channel hash not required right now, will be used for seeding/namespaces later on.
    uint64_t _channel_hash = 0;
    VW::string_view feature_name = feature->name()->c_str();
    // Hash feature name
    word_hash = (all->p->hasher(feature_name.begin(), feature_name.length(), _channel_hash));
    // Push into features
    fs.push_back(feature->value(), word_hash);
}
} // VW