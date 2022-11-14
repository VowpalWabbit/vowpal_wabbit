#include "auto_flatbuf.h"

#include <sstream>

#include "type_registry.h"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/bfbs_generator.h"
#include "flatbuffers/util.h"

using namespace type_system;
using std::stringstream;
using namespace auto_flatbuf;

struct schema_builder_ctx
{
  const schema_descriptor& descriptor;
  const type_system::type_registry& registry;
};

struct fb_table_definition
{
  std::string ns;
  std::string name;

  bool is_empty;
  std::stringstream table_builder;

  std::vector<std::string> type_imports;
};

fb_table_definition build_table_for_type(schema_builder_ctx ctx, const type_descriptor& ti)
{
  assert(!ti.is_builtin);

  std::vector<std::string> type_imports;
  stringstream table_builder;

  if (!ti.is_empty())
  {
    table_builder << "table " << ti.name << " {" << std::endl;

    std::for_each(ti.properties_begin(), ti.properties_end(),
    [&ti, &table_builder, &type_imports, &ctx](auto& it)
    {
      const property_descriptor& pi = it.second;
      table_builder << "  " << pi.name << ": ";

      if (pi.is_vector) { table_builder << "["; }

      auto maybe_pti = ctx.registry.find_type(pi.etype.tindex);
      
      if (maybe_pti != ctx.registry.types_end())
      {
        const type_descriptor& pti = *maybe_pti;
        
        
        if (pti.is_empty())
        {
          table_builder << "bool";
        }
        else
        {
          if (!pti.is_builtin)
          {
            type_imports.push_back(pti.name);
          }

          table_builder << pti.name;
        }
      }
      else 
      { 
        //TODO: how do we report errors?!
        table_builder << "<ERROR-TYPE( for " << ctx.descriptor.make_qualified_name(ti.name) << "." << pi.name << " )>";
      }

      if (pi.is_vector) { table_builder << "]"; }

      table_builder << ";" << std::endl;
    });

    table_builder << "}" << std::endl;
  }

  return { ctx.descriptor.schema_namespace, ti.name, ti.is_empty(), std::move(table_builder), type_imports };
}

namespace auto_flatbuf
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
    static_assert(sizeof(std::remove_pointer_t<decltype(buf)>) == sizeof(char), 
        "fb buffer type is not the same as char*");
        
    std::string result;
    result.resize(size);

    memcpy_s(&result[0], result.size(), buf, size);

    return { result };
  }

  fbs_data schema_builder::build_idl()
  {
    schema_builder_ctx ctx = { descriptor, type_registry::instance() };
    stringstream schema_builder;

    //std::vector<fb_table_definition> tables;
    schema_builder << "namespace " << descriptor.schema_namespace << ";" << std::endl << std::endl;

    // we assume, in schema_builder, that our type_registry is transitively closed under property chasing
    // this means that we can assume that all types referenced by properties are in the registry, or are
    // builtins
    std::for_each(ctx.registry.types_begin(), ctx.registry.types_end(),
    [&schema_builder, &ctx](auto& it)
    {
      const type_descriptor& ti = it;
      if (!ti.is_builtin)
      {
        if (ti.has_activator()) { activation a = ti.activate(); }

        if (ti.is_empty())
        {
          // if (!ti.has_activator) { } //report an error here?
          return;
        }

        fb_table_definition table = build_table_for_type(ctx, ti);

        schema_builder << table.table_builder.str() << std::endl;
      }
    });

    return { schema_builder.str() };
  }
}