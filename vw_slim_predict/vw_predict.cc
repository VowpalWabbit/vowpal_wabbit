#include "vw_predict.h"
#include "gd_predict.h"

#include <stdexcept>
#include <iostream>
#include <iomanip>

using namespace std;

// #define MODEL_PARSER_DEBUG

class model_parser
{
  const char* _model_begin;
  const char* _model;
  const char* _model_end;

public:
  model_parser(const char* model, size_t length)
    : _model_begin(model), _model(model), _model_end(model + length)
  { }

  const char* read(const char* field_name, size_t field_length)
  { // check if we're inside the buffer
    const char* new_model = _model + field_length;
    if (new_model > _model_end)
      throw std::length_error(field_name);

    const char* ret = _model;
    // advance begin
    _model = new_model;

    return ret;
  }

  template<typename T>
  T read(const char* field_name)
  {
#ifdef MODEL_PARSER_DEBUG
    cout << setw(4) << std::hex << (_model - _model_begin) << std::dec;
#endif

    T temp = *(T*)read(field_name, sizeof(T));

#ifdef MODEL_PARSER_DEBUG
    cout << setw(18) << field_name << " '" << temp << '\'' << endl;
#endif

    return temp;
  }

  string read_string(const char* field_name)
  {
    uint32_t str_len = read<uint32_t>("string.len");
    string s(read(field_name, str_len), str_len-1);

#ifdef MODEL_PARSER_DEBUG
    cout << setw(18) << field_name << " '" << s << '\'' << endl;
#endif

    return s;
  }

  void skip(size_t bytes)
  {
    _model += bytes;
    if (_model > _model_end)
      throw std::length_error("skip");
  }

  template<typename T>
  void read_dense_weights(unique_ptr<dense_parameters>& weights, uint64_t weight_length)
  {
    while (_model < _model_end)
    {
      T idx = read<T>("gd.weight.index");
      if (idx > weight_length)
        throw std::invalid_argument("gd.stored_weight_length must small than 2^num_bits");

      weights->strided_index(idx) = read<weight>("gd.weight.value");
    }
  }
};

vw_predict::vw_predict(const char* model, size_t length)
{
  // required for inline_predict
  memset(_ignore_linear, false, sizeof(_ignore_linear));

  model_parser mp(model, length);

  // parser_regressor.cc: save_load_header
  _version = mp.read_string("version");

  // read model id
  _id = string(mp.read_string("model_id"));

  mp.read<char>("model character");
  mp.read<float>("min_label");
  mp.read<float>("max_label");

  uint32_t num_bits = mp.read<uint32_t>("num_bits");
  uint64_t weight_length = (uint64_t)1 << num_bits;
  _weights = std::unique_ptr<dense_parameters>(new dense_parameters(weight_length));

  mp.read<uint32_t>("lda");

  uint32_t ngram_len = mp.read<uint32_t>("ngram_len");
  mp.skip(3 * ngram_len);

  uint32_t skips_len = mp.read<uint32_t>("skips_len");
  mp.skip(3 * skips_len);

  mp.read_string("file_options");

  if (mp.read<uint32_t>("check_sum_len") != 4)
    throw std::invalid_argument("check_sum_len must be 4");
  mp.read<uint32_t>("check_sum");

  // gd.cc: save_load
  if (mp.read<bool>("gd.resume"))
    throw std::invalid_argument("gd.resume must be false");

  // read sparse weights into dense
  if (num_bits < 31)
    mp.read_dense_weights<uint32_t>(_weights, weight_length);
  else
    mp.read_dense_weights<uint64_t>(_weights, weight_length);

  // check that permutations is not enabled (or parse it)
}

void vw_predict::score(example_predict* shared, std::vector<example_predict*> actions, std::vector<float>& out_scores)
{
  safe_example_predict ex;
  GD::inline_predict<dense_parameters, vector<string>>(*_weights, false, _ignore_linear, _interactions, /* permutations */ false, ex);
}
