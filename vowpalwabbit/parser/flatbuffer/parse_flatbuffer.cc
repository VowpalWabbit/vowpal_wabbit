#include <fstream>
#include <iostream>

#include "parse_flatbuffer.h"
#include "test_generated.h"
#include "global_data.h"
#include "example.h"
#include "constant.h"

namespace VW
{
void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples)
{
  all->trace_message << "Read00\n";
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
  // all->trace_message << index << "\n";
  all->max_examples = data->examples()->size();
  parse_flatbuf_examples(all, examples, data);
  // std::cout << "Example indices " << examples[0]->indices.size();
  // std::cout << "Feature indices " << examples[0]->feature_space[(unsigned char)' '].indicies.size() << " values " << examples[0]->feature_space[(unsigned char)' '].values.size() << std::endl;

  return 31; // Get rid of this
}

void parse_flatbuf_examples(vw* all, v_array<example*>& examples, const Data* data)
{
  parse_flatbuf_example(all, examples[0], data->examples()->Get(index));
  index++;
}

void parse_flatbuf_example(vw* all, example* ae, const Datapoint* datapoint)
{
  all->p->lp.default_label(&ae->l);
  
  ae->l.simple.label = (float)datapoint->label();

  unsigned char _index = (unsigned char)' ';
  bool _new_index = false;
  if (ae->feature_space[_index].size() == 0)
      _new_index = true;

  features& fs = ae->feature_space[_index];

  // all->trace_message << "Number of features " << datapoint->features()->size() << "\n";

  for (int j=0; j<datapoint->features()->size(); j++)
  {
      parse_flatbuf_features(all, ae, fs, datapoint->features()->Get(j), _index);
  }

  // all->trace_message << "Read features " << fs.values.size() << "\n";
  // all->trace_message << "Read indices " << fs.indicies.size() << "\n";

  // if (_new_index && ae->feature_space[_index].size() > 0)
  ae->indices.push_back(_index);

}

void parse_flatbuf_features(vw* all, example* ae, features& fs, const Feature* feature, unsigned char _index)
{
  uint64_t word_hash;
  // // Channel hash not required right now, will be used for seeding/namespaces later on.
  uint64_t _channel_hash = 0;
  VW::string_view feature_name(feature->name()->c_str());
  // // Hash feature name
  word_hash = (all->p->hasher(feature_name.begin(), feature_name.length(), _channel_hash));
  // Push into features
  fs.push_back(feature->value(), word_hash);
  feature_value _v = feature->value();
  // std::cout << _v << "\n";
  v_array<char> _spelling;
  _spelling = v_init<char>();

  if (((all->affix_features)[_index] > 0) && (!feature_name.empty()))
  {
    features& affix_fs = ae->feature_space[affix_namespace];
    if (affix_fs.size() == 0)
      ae->indices.push_back(affix_namespace);
    uint64_t affix = (all->affix_features)[_index];

    while (affix > 0)
    {
      bool is_prefix = affix & 0x1;
      uint64_t len = (affix >> 1) & 0x7;
      VW::string_view affix_name(feature_name);
      if (affix_name.size() > len)
      {
        if (is_prefix)
          affix_name.remove_suffix(affix_name.size() - len);
        else
          affix_name.remove_prefix(affix_name.size() - len);
      }

      word_hash = all->p->hasher(affix_name.begin(), affix_name.length(), (uint64_t)_channel_hash) * (affix_constant + (affix & 0xF) * quadratic_constant);
      affix_fs.push_back(_v, word_hash);
      if (all->audit)
      {
        v_array<char> affix_v = v_init<char>();
        if (_index != ' ')
          affix_v.push_back(_index);
        affix_v.push_back(is_prefix ? '+' : '-');
        affix_v.push_back('0' + (char)len);
        affix_v.push_back('=');
        push_many(affix_v, affix_name.begin(), affix_name.size());
        affix_v.push_back('\0');
        affix_fs.space_names.push_back(audit_strings_ptr(new audit_strings("affix", affix_v.begin())));
      }
      affix >>= 4;
    }
  }
  if ((all->spelling_features)[_index])
  {
    features& spell_fs = ae->feature_space[spelling_namespace];
    if (spell_fs.size() == 0)
      ae->indices.push_back(spelling_namespace);
    // v_array<char> spelling;
    _spelling.clear();
    for (char c : feature_name)
    {
      char d = 0;
      if ((c >= '0') && (c <= '9'))
        d = '0';
      else if ((c >= 'a') && (c <= 'z'))
        d = 'a';
      else if ((c >= 'A') && (c <= 'Z'))
        d = 'A';
      else if (c == '.')
        d = '.';
      else
        d = '#';
      // if ((spelling.size() == 0) || (spelling.last() != d))
      _spelling.push_back(d);
    }

    VW::string_view spelling_strview(_spelling.begin(), _spelling.size());
    uint64_t word_hash = hashstring(spelling_strview.begin(), spelling_strview.length(), (uint64_t)_channel_hash);
    spell_fs.push_back(_v, word_hash);
    if (all->audit)
    {
      v_array<char> spelling_v = v_init<char>();
      if (_index != ' ')
      {
        spelling_v.push_back(_index);
        spelling_v.push_back('_');
      }
      push_many(spelling_v, spelling_strview.begin(), spelling_strview.size());
      spelling_v.push_back('\0');
      spell_fs.space_names.push_back(audit_strings_ptr(new audit_strings("spelling", spelling_v.begin())));
    }
  }
  if ((all->namespace_dictionaries)[_index].size() > 0)
  {
    // Heterogeneous lookup not yet implemented in std
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r0.html
    const std::string feature_name_str(feature_name);
    for (const auto& map : (all->namespace_dictionaries)[_index])
    {
      const auto& feats_it = map->find(feature_name_str);
      if ((feats_it != map->end()) && (feats_it->second->values.size() > 0))
      {
        const auto& feats = feats_it->second;
        features& dict_fs = ae->feature_space[dictionary_namespace];
        if (dict_fs.size() == 0)
          ae->indices.push_back(dictionary_namespace);
        push_many(dict_fs.values, feats->values.begin(), feats->values.size());
        push_many(dict_fs.indicies, feats->indicies.begin(), feats->indicies.size());
        dict_fs.sum_feat_sq += feats->sum_feat_sq;
        if (all->audit)
          for (const auto& id : feats->indicies)
          {
            std::stringstream ss;
            ss << _index << '_';
            ss << feature_name;
            ss << '=' << id;
            dict_fs.space_names.push_back(audit_strings_ptr(new audit_strings("dictionary", ss.str())));
          }
      }
    }
  }
}
} // VW