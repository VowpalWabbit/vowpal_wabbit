#include "vw/slim/model_parser.h"

namespace vw_slim
{
model_parser::model_parser(const char* model, size_t length)
    :
#ifdef MODEL_PARSER_DEBUG
    _model_begin(model)
    ,
#endif
    _model(model)
    , _model_end(model + length)
    , _checksum(0)
{
#ifdef MODEL_PARSER_DEBUG
  std::cout << "moder_parser("
            << "0x" << std::hex << std::setw(8) << (uint64_t)_model_begin << "-" << std::hex << std::setw(8)
            << (uint64_t)_model_end << ")" << std::dec;
#endif
}

const char* model_parser::position() { return _model; }

uint32_t model_parser::checksum() { return _checksum; }

int model_parser::read(const char* field_name, size_t field_length, const char** ret)
{
  // Silence unused warning - Only used in debug mode
  ((void)(field_name));

  // check if we're inside the buffer
  const char* new_model = _model + field_length;
  if (new_model > _model_end) { return E_VW_PREDICT_ERR_INVALID_MODEL; }

#ifdef MODEL_PARSER_DEBUG
  std::fstream log("vwslim-debug.log", std::fstream::app);
  log << "reading " << field_name << std::endl;
#endif

  *ret = _model;
  // advance begin
  _model = new_model;

  return S_VW_PREDICT_OK;
}

int model_parser::skip(size_t bytes)
{
  const char* new_model = _model + bytes;
  if (new_model > _model_end) { return E_VW_PREDICT_ERR_INVALID_MODEL; }

  if (bytes > 0) { _checksum = (uint32_t)VW::uniform_hash(_model, bytes, _checksum); }

  _model = new_model;

  return S_VW_PREDICT_OK;
}
}  // namespace vw_slim
