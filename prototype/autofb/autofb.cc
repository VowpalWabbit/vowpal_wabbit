#include <iostream>
#include <unordered_map>

#include "autofb.h"

using namespace typesys;
using std::cout;
using std::endl;



using type_map = std::unordered_map<std::type_index, std::string>;
using ts_type_map = std::unordered_map<typesys::type_index, std::string>;

static const type_map flatbuffers_typemap = {
    { typeid(std::int8_t), "int8" },
    { typeid(std::int16_t), "int16" },
    { typeid(std::int32_t), "int32" },
    { typeid(std::uint8_t), "uint8" },
    { typeid(std::uint16_t), "uint16" },
    { typeid(std::uint32_t), "uint32" },
    { typeid(std::uint64_t), "uint64" },
    { typeid(float), "float" },
    { typeid(double), "double" },
    { typeid(std::string), "string" },
    { typeid(bool), "bool" },
  }; 

void print_schema_header(const std::string& ns, std::ostream& os = cout)
{
  os << "namespace " << ns << ';' << endl;
}

void print_schema(
  const type_map& fb_typemap,
  ts_type_map& schema_typemap,
  const typesys::type_info& ti,
  std::ostream& os = cout
)
{
  using std::endl;

  if (ti.has_base())
  {
    // find the include schema path
    using iter = ts_type_map::const_iterator;
    iter it = schema_typemap.find(ti.base());
    
    // if not found
    if (it == schema_typemap.end())
    {
      versioned_name vn = universe::instance().get_type(ti.base()).name;
      os << "//include \"$(" << vn << ")\"; // path not available!" << endl; 
    }
    else
    {
      os << "include \"" << schema_typemap[ti.base()] << "\";" << endl;
    }

    // TODO: Rest of includes
    os << endl;
  }

  // TODO: this should really also support typed properties
  // But we need to enlighten the property system to reflectable
  // types.

  os << "table " << ti.name;
  
  if (ti.has_base())
  {
    // lookup the type info for base based on the typeindex in ti
    const typesys::type_info& base_ti = typesys::universe::instance().get_type(ti.base());
    os << " : " << base_ti.name;
  }

  os << " {" << endl;
  std::for_each(ti.props_begin(), ti.props_end(), [&fb_typemap, &os](const typesys::property_info& pi) {
    // find the appropriate mapping
    type_map::const_iterator it = fb_typemap.find(pi.etype.tindex);
    os << pi.name;

    if (it == fb_typemap.end())
    {
      // TODO: ERROR!
      os << " : <error-type>;" << endl;
    }
    else
    {
      os << " : " << it->second << ';' << endl;
    }
  });
  os << "}" << endl;
}

template <bool write_schema_to_cout = false>
void generate_universe_types_internal(
    const std::string& schema_ns, 
    const std::string& output_dir,
    universe& u
  )
{
  ts_type_map schema_map;

  std::for_each(u.types_begin(), u.types_end(), 
    [&schema_map, &schema_ns, &output_dir]
    (const typesys::type_info& ti) 
    {
      ti.activate();
      schema_map[ti.index] = std::string(ti.name.name) + ".fbs";
      const std::string schema_path = output_dir + schema_map[ti.index];
      
      std::cout << "Generating schema for " << ti.name;
      if (!write_schema_to_cout) std::cout << " to " << schema_path;
      if (ti.props_end() == ti.props_begin())
      {
        std::cout << ". No properties (empty type): Skipping." << std::endl;
        return;
      }
      else
      {
        std::cout << "..." << std::endl;
      }
      
      std::fstream* fs = nullptr;
      if (!write_schema_to_cout)
      {
        fs = new std::fstream();
        fs->open(schema_path, std::ios::out);
      }

      std::ostream& os = write_schema_to_cout ? std::cout : *fs;
      print_schema_header(schema_ns, os);
      os << std::endl;

      print_schema(flatbuffers_typemap, schema_map, ti, os);
      
      if (!write_schema_to_cout)
      {
        fs->flush();
        fs->close();
        delete fs;
      }
    });
}

namespace autofb
{
  void generate_universe_types(
    const std::string& schema_ns, 
    const std::string& output_dir,
    universe& types
  ) 
  {
    generate_universe_types_internal<false>(schema_ns, output_dir, types);
  }

  void print_universe_types(
    const std::string& schema_ns, 
    universe& types
  )
  {
    generate_universe_types_internal<true>(schema_ns, "", types);
  }
}