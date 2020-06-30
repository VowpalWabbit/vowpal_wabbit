#include <fstream>
#include <iostream>
#include <cfloat>

#include "../../global_data.h"
#include "../../example.h"
#include "../../constant.h"
#include "../../cb.h"
#include "../../action_score.h"
#include "../../best_constant.h"
#include "parse_flat_example.h"
#include "generated/example_generated.h"

namespace VW {
namespace parsers {
namespace flatbuffer {

parser::parser(std::string filename) : 
_filename(filename),
_example_index(0),
_c_hash(0)
//_flatbuffer_pointer()?
{
  // _filename = filename;
  init(); //Shift as a separate call
}
parser::parser(uint8_t *buffer_pointer) : 
_flatbuffer_pointer(buffer_pointer),
_example_index(0),
_c_hash(0),
_filename("")
{
  _data = VW::parsers::flatbuffer::GetExampleCollection(_flatbuffer_pointer); //Shift as a separate call
}

void parser::init()
{
  std::ifstream infile;
  infile.open(_filename, std::ios::binary | std::ios::in);
  if (!infile.good()) THROW_EX(VW::vw_argument_invalid_value_exception, "Flatbuffer does not exist");

  infile.seekg(0,std::ios::end);
  int length = infile.tellg();
  infile.seekg(0,std::ios::beg);
  std::unique_ptr<char> buffer_pointer(new char[length]);
  infile.read(buffer_pointer.get(), length);
  _flatbuffer_pointer = reinterpret_cast<u_int8_t*>(buffer_pointer.get());

  _data = VW::parsers::flatbuffer::GetExampleCollection(_flatbuffer_pointer);
}

const VW::parsers::flatbuffer::ExampleCollection* parser::data() {return _data;}

void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples)
{
  // Do something
}

int flatbuffer_to_examples(vw* all, v_array<example*>& examples)
{
  return (int) all->flat_converter->parse_examples(all, examples); // Get rid of this
}

bool parser::parse_examples(vw* all, v_array<example*>& examples)
{
  if (!flatbuffers::IsFieldPresent(_data, ExampleCollection::VT_EXAMPLES))
  {
    THROW("No examples to parse");
  }
  if (_example_index == _data->examples()->size() - 1)
    return false;
  parse_example(all, examples[0], _data->examples()->Get(_example_index));
  _example_index++;
  return true;
}

void parser::parse_example(vw* all, example* ae, const Example* eg)
{
  all->p->lp.default_label(&ae->l);
  parse_flat_label(all, ae, eg);

  if (flatbuffers::IsFieldPresent(eg, Example::VT_TAG)){
    ae->tag = v_init<char>();
    VW::string_view tag(eg->tag()->c_str());
    push_many(ae->tag, tag.begin(), tag.size());
  }

  for (int i=0; i < eg->namespaces()->size(); i++){
    parse_namespaces(all, ae, eg->namespaces()->Get(i));
  }
}

void parser::parse_namespaces(vw* all, example* ae, const Namespace* ns)
{

  if (flatbuffers::IsFieldPresent(ns, Namespace::VT_NAME)){
    ae->indices.push_back(ns->name()->c_str()[0]);
    _c_hash = all->p->hasher(ns->name()->c_str(), ns->name()->Length(), all->hash_seed);
  }
  else 
  {ae->indices.push_back(ns->hash());}

  features& fs = ae->feature_space[ns->hash()];

  for (int j=0; j<ns->features()->size(); j++){
    parse_features(all, ae, fs, ns->features()->Get(j));
  }
}

void parser::parse_features(vw* all, example* ae, features& fs, const Feature* feature)
{
  if (flatbuffers::IsFieldPresent(feature, Feature::VT_NAME)){
    uint64_t word_hash = all->p->hasher(feature->name()->c_str(), feature->name()->Length(), _c_hash);
    fs.push_back(feature->value(), word_hash);
  }
  else 
  {fs.push_back(feature->value(), feature->hash());}
}

void parser::parse_flat_label(vw* all, example* ae, const Example* eg)
{
  Label label_type = eg->label_type();

  switch (label_type)
  {
  case Label_SimpleLabel:{
    const SimpleLabel* simple_label = static_cast<const SimpleLabel*>(eg->label());
    parse_simple_label(all->sd, &(ae->l), simple_label);
    break;
  }
  case Label_CBLabel:{
    const CBLabel* cb_label = static_cast<const CBLabel*>(eg->label());
    parse_cb_label(&(ae->l), cb_label);
    break;
  }
  case Label_CCBLabel:{
    const CCBLabel* ccb_label = static_cast<const CCBLabel*>(eg->label());
    parse_ccb_label(&(ae->l), ccb_label);
    break;
  }
  case Label_CB_EVAL_Label:{
    auto cb_eval_label = static_cast<const CB_EVAL_Label*>(eg->label());
    parse_cb_eval_label(&(ae->l), cb_eval_label);
    break;
  }
  case Label_CS_Label:{
    auto cs_label = static_cast<const CS_Label*>(eg->label());
    parse_cs_label(&(ae->l), cs_label);
    break;
  }
  case Label_MultiClass:{
    auto mc_label = static_cast<const MultiClass*>(eg->label());
    parse_mc_label(all->sd, &(ae->l), mc_label);
    break;
  }
  case Label_MultiLabel:{
    auto multi_label = static_cast<const MultiLabel*>(eg->label());
    parse_multi_label(&(ae->l), multi_label);
    break;
  }
  case Label_Slates_Label:{
    auto slates_label = static_cast<const Slates_Label*>(eg->label());
    parse_slates_label(&(ae->l), slates_label);
    break;
  }
  case Label_no_label:
    // auto label = static_cast<const no_label*>(eg->label());
    parse_no_label();
    break;
  default:
    THROW("Label type in Flatbuffer not understood");
  }
}

} //flatbuffer
} // parser
} // VW
