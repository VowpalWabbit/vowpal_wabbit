#include "autofb_schema.h"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/bfbs_generator.h"
#include "flatbuffers/util.h"

namespace autofb
{
bfbs_data fbs_data::to_binary()
{
  flatbuffers::IDLOptions opts;

  flatbuffers::Parser parser;
  if (!parser.Parse(text_data.c_str()))
  {
    // TODO: ERROR!
    std::string last_error = "Failed to parse schema: " + parser.error_;
    std::cout << last_error << std::endl << std::endl;
    std::cout << text_data << std::endl;

    throw new std::exception(last_error.c_str());
  }

  parser.Serialize();

  auto buf = parser.builder_.GetBufferPointer();
  auto size = parser.builder_.GetSize();

  // The assertion here is simply to flag a situation in which the fb API changes somehow so that its
  // buffer representation is no longer conveniently put into a std::string. (We should probably be
  // using vectors instead, honestly, but this is based on the reflection examples.)
  static_assert(
      sizeof(std::remove_pointer_t<decltype(buf)>) == sizeof(char), "fb buffer type is not the same as char*");

  std::string result;
  result.resize(size);

  memcpy_s(&result[0], result.size(), buf, size);

  return {result};
}
}