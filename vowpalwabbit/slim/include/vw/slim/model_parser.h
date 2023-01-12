#pragma once

#include "vw/common/hash.h"
#include "vw_slim_return_codes.h"

#include <cctype>
#include <memory>
#include <string>

// #define MODEL_PARSER_DEBUG

#ifdef MODEL_PARSER_DEBUG
#  include <fstream>
#  include <iomanip>
#  include <iostream>
#endif

namespace vw_slim
{
class model_parser
{
public:
  model_parser(const char* model, size_t length);

  int read(const char* field_name, size_t field_length, const char** ret);

  int skip(size_t bytes);

  const char* position();

  uint32_t checksum();

  template <bool compute_checksum>
  int read_string(const char* field_name, std::string& s)
  {
    uint32_t str_len;
    RETURN_ON_FAIL((read<uint32_t, compute_checksum>("string.len", str_len)));
#ifdef MODEL_PARSER_DEBUG
    {
      std::fstream log("vwslim-debug.log", std::fstream::app);
      log << std::setw(18) << field_name << " length " << str_len << std::endl;
    }
#endif

    // 0 length strings are not valid, need to contain at least \0
    if (str_len == 0) { return E_VW_PREDICT_ERR_INVALID_MODEL; }

    const char* data;
    RETURN_ON_FAIL(read(field_name, str_len, &data));

    s = std::string(data, str_len - 1);
#ifdef MODEL_PARSER_DEBUG
    {
      std::fstream log("vwslim-debug.log", std::fstream::app);
      log << std::setw(18) << field_name << " '" << s << '\'' << std::endl;
    }
#endif

    // calculate checksum
    if (compute_checksum && str_len > 0) { _checksum = (uint32_t)VW::uniform_hash(data, str_len, _checksum); }

    return S_VW_PREDICT_OK;
  }

  template <typename T, bool compute_checksum>
  int read(const char* field_name, T& val)
  {
#ifdef MODEL_PARSER_DEBUG
    std::fstream log("vwslim-debug.log", std::fstream::app);
    log << std::setw(18) << field_name << " 0x" << std::hex << std::setw(8) << (uint64_t)_model << "-" << std::hex
        << std::setw(8) << (uint64_t)_model_end << " " << std::setw(4) << (_model - _model_begin)
        << " field: " << std::setw(8) << (uint64_t)&val << std::dec;
#endif

    const char* data;
    RETURN_ON_FAIL(read(field_name, sizeof(T), &data));

    // avoid alignment issues for 32/64bit types on e.g. Android/ARM
    memcpy(&val, data, sizeof(T));

    if (compute_checksum) { _checksum = VW::uniform_hash(reinterpret_cast<const char*>(&val), sizeof(T), _checksum); }

#ifdef MODEL_PARSER_DEBUG
    log << " '" << val << '\'' << std::endl;
#endif

    return S_VW_PREDICT_OK;
  }

  // default overload without checksum hashing
  template <typename T>
  int read(const char* field_name, T& val)
  {
    return read<T, true>(field_name, val);
  }

  template <typename T, typename W>
  int read_weights(std::unique_ptr<W>& weights, uint64_t weight_length)
  {
    // weights are excluded from checksum calculation
    while (_model < _model_end)
    {
      T idx;
      RETURN_ON_FAIL((read<T, false>("gd.weight.index", idx)));
      if (idx > weight_length) { return E_VW_PREDICT_ERR_WEIGHT_INDEX_OUT_OF_RANGE; }

      float& w = (*weights)[static_cast<size_t>(idx)];
      RETURN_ON_FAIL((read<float, false>("gd.weight.value", w)));

#ifdef MODEL_PARSER_DEBUG
      std::cout << "weight. idx: " << idx << ":" << (*weights)[idx] << std::endl;
#endif
    }

    return S_VW_PREDICT_OK;
  }

  template <typename W>
  int read_weights(std::unique_ptr<W>& weights, uint32_t num_bits, uint32_t stride_shift)
  {
    auto weight_length = static_cast<size_t>(uint64_t{1} << num_bits);

    weights = std::unique_ptr<W>(new W(weight_length));
    weights->stride_shift(stride_shift);

    if (num_bits < 31) { RETURN_ON_FAIL((read_weights<uint32_t, W>(weights, weight_length))); }
    else
    {
      // Can't load a 64 bit model on 32 bit arch.
      if (sizeof(size_t) == 4) { return E_VW_PREDICT_ERR_INVALID_MODEL; }

      RETURN_ON_FAIL((read_weights<uint64_t, W>(weights, weight_length)));
    }

    return S_VW_PREDICT_OK;
  }

private:
#ifdef MODEL_PARSER_DEBUG
  const char* _model_begin;
#endif
  const char* _model;
  const char* _model_end;
  uint32_t _checksum;
};
}  // namespace vw_slim
