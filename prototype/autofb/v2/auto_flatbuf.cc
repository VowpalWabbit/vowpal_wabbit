#include "auto_flatbuf.h"

#include <sstream>

#include "type_registry.h"
#include "type_construtors.h"
#include "type_erase.h"

namespace auto_flatbuf
{

using namespace type_system;
using std::cout;
using std::endl;

using std_type_map = std::unordered_map<std::type_index, std::string>;
using ts_type_map = std::unordered_map<type_number, std::string>;

static const std_type_map flatbuffers_typemap = {
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



const type_descriptor& lookup_base(const type_descriptor& td)
{
  if (!td.has_base()) return td;

  return type_system::type_registry::instance().lookup_type(td.get_base_index());
}

inline type_registry::type_iter lookup_type(const std::type_index ti)
{
  return type_registry::instance().find_type(ti);
}

void print_schema(
  const std_type_map& fb_typemap,
  const std::string& schema_namespace,
  ts_type_map& schema_typemap,
  const type_system::type_descriptor& ti,
  std::ostream& os = cout
)
{
  using std::endl;
  std::stringstream temp;

  temp << "namespace " << schema_namespace << ';' << endl;

  if (ti.has_base())
  {
    // find the include schema path
    using iter = ts_type_map::const_iterator;
    iter it = schema_typemap.find(ti.get_base_index());
    
    // if not found
    if (it == schema_typemap.end())
    {
      name_t name = lookup_base(ti).name;
      os << "//include \"$(" << name << ")\"; // path not available!" << endl; 
    }
    else
    {
      os << "include \"" << schema_typemap[ti.get_base_index()] << "\";" << endl;
    }
  }

  // TODO: this should really also support typed properties
  // But we need to enlighten the property system to reflectable
  // types.

  temp << "table " << ti.name;
  
  if (ti.has_base())
  {
    // lookup the type info for base based on the typeindex in ti
    const type_system::type_descriptor& base_ti = lookup_base(ti);
    temp << " : " << base_ti.name;
  }

  temp << " {" << endl;

  using named_property = type_system::type_descriptor::name_property;

  std::for_each(ti.properties_begin(), ti.properties_end(), [&fb_typemap, &temp, &schema_typemap, &os](const named_property& np) {
    // find the appropriate mapping
    const property_descriptor& pi = np.second;
    std_type_map::const_iterator it = fb_typemap.find(pi.etype.tindex);
    temp << pi.name;

    std::string property_type = "<error-type>";

    if (it == fb_typemap.end())
    {
      // Check if this is a reflectable type
      auto it2 = lookup_type(pi.etype.tindex);
      if (it2 != type_registry::instance().types_end())
      {
        const type_descriptor& td = *it2;
        os << "include \"" << schema_typemap[td.typeindex] << "\";" << endl;

        property_type = td.name;
      }
    }
    else
    {
      property_type = it->second;
    }

    if (pi.is_vector)
    {
      temp << " : [" << property_type << "];" << endl;
    }
    else
    {
      temp << " : " << property_type << ';' << endl;
    }
  });

  os << endl << temp.str() << "}" << endl;
}

template <bool write_schema_to_cout = false>
void generate_universe_types_internal(
    const std::string& schema_ns, 
    const std::string& output_dir,
    type_registry& u
  )
{
  ts_type_map schema_map;

  std::for_each(u.types_begin(), u.types_end(), 
    [&schema_map, &schema_ns, &output_dir]
    (const type_descriptor& ti) 
    {
      std::cout << "Generating schema for " << ti.name;
      
      if (ti.is_builtin)
      {
        std::cout << ". Built-in type, skipping." << std::endl;
        return;
      }

      if (!ti.has_activator())
      {
        std::cout << ". No activator, skipping." << std::endl;
        return;
      }

      ti.activate();
      schema_map[ti.typeindex] = std::string(ti.name) + ".fbs";
      const std::string schema_path = output_dir + schema_map[ti.typeindex];
      
      if (!write_schema_to_cout) std::cout << " to " << schema_path;
      if (ti.properties_end() == ti.properties_begin())
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
      //print_schema_header(schema_ns, os);
      //os << std::endl;

      print_schema(flatbuffers_typemap, schema_ns, schema_map, ti, os);
      
      if (!write_schema_to_cout)
      {
        fs->flush();
        fs->close();
        delete fs;
      }
    });
}

  void generate_universe_types(
    const std::string& schema_ns, 
    const std::string& output_dir,
    type_registry& types
  ) 
  {
    generate_universe_types_internal<false>(schema_ns, output_dir, types);
  }

  void print_universe_types(
    const std::string& schema_ns, 
    type_registry& types
  )
  {
    generate_universe_types_internal<true>(schema_ns, "", types);
  }

  struct fbb_AddElement_dispatcher
{
  #define __FBB_ADD_ELEMENT_DISPATCHER_PACK flatbuffers::FlatBufferBuilder&, const reflection::Field&, erased_lvalue&
  using dispatch_f = ::dispatch_f<__FBB_ADD_ELEMENT_DISPATCHER_PACK>;
  using dispatch_table = erased_dispatch_table<__FBB_ADD_ELEMENT_DISPATCHER_PACK>;

  fbb_AddElement_dispatcher()
  {
    dt.add<std::int8_t, &fbb_AddElement_dispatcher::dispatcher<std::int8_t>>()
      .add<std::int16_t, &fbb_AddElement_dispatcher::dispatcher<std::int16_t>>()
      .add<std::int32_t, &fbb_AddElement_dispatcher::dispatcher<std::int32_t>>()
      .add<std::int64_t, &fbb_AddElement_dispatcher::dispatcher<std::int64_t>>()
      .add<std::uint8_t, &fbb_AddElement_dispatcher::dispatcher<std::uint8_t>>()
      .add<std::uint16_t, &fbb_AddElement_dispatcher::dispatcher<std::uint16_t>>()
      .add<std::uint32_t, &fbb_AddElement_dispatcher::dispatcher<std::uint32_t>>()
      .add<std::uint64_t, &fbb_AddElement_dispatcher::dispatcher<std::uint64_t>>()
      .add<float, &fbb_AddElement_dispatcher::dispatcher<float>>()
      .add<double, &fbb_AddElement_dispatcher::dispatcher<double>>()
      .add<bool, &fbb_AddElement_dispatcher::dispatcher<bool>>();
  }

  void operator()(flatbuffers::FlatBufferBuilder& fbb, const reflection::Field& field, erased_lvalue& value)
  {
    dt.dispatch(value._type, fbb, field, value);
  }

private:
  dispatch_table dt;

  template <typename T>
  static void dispatcher(flatbuffers::FlatBufferBuilder& fbb, const reflection::Field& field, erased_lvalue& erased_lvalue)
  {
    fbb.AddElement<T>(field.offset(), erased_lvalue.get<T>());
  }
};

struct fbb_ReadElement_dispatcher
{
  #define __FBB_READ_ELEMENT_DISPATCHER_PACK const flatbuffers::Table&, const reflection::Field&, erased_lvalue&
  using dispatch_f = ::dispatch_f<__FBB_READ_ELEMENT_DISPATCHER_PACK>;
  using dispatch_table = erased_dispatch_table<__FBB_READ_ELEMENT_DISPATCHER_PACK>;

  fbb_ReadElement_dispatcher()
  {
    dt.add<std::int8_t, &fbb_ReadElement_dispatcher::dispatcher<std::int8_t>>()
      .add<std::int16_t, &fbb_ReadElement_dispatcher::dispatcher<std::int16_t>>()
      .add<std::int32_t, &fbb_ReadElement_dispatcher::dispatcher<std::int32_t>>()
      .add<std::int64_t, &fbb_ReadElement_dispatcher::dispatcher<std::int64_t>>()
      .add<std::uint8_t, &fbb_ReadElement_dispatcher::dispatcher<std::uint8_t>>()
      .add<std::uint16_t, &fbb_ReadElement_dispatcher::dispatcher<std::uint16_t>>()
      .add<std::uint32_t, &fbb_ReadElement_dispatcher::dispatcher<std::uint32_t>>()
      .add<std::uint64_t, &fbb_ReadElement_dispatcher::dispatcher<std::uint64_t>>()
      .add<float, &fbb_ReadElement_dispatcher::dispatcher<float>>()
      .add<double, &fbb_ReadElement_dispatcher::dispatcher<double>>()
      .add<bool, &fbb_ReadElement_dispatcher::dispatcher<bool>>();
  }

  void operator()(const flatbuffers::Table& table, const reflection::Field& field, erased_lvalue& value)
  {
    dt.dispatch(value._type, table, field, value);
  }

private:
  dispatch_table dt;

  template <typename T>
  static void dispatcher(const flatbuffers::Table& table, const reflection::Field& field, erased_lvalue& erased_lvalue)
  {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");

    if (std::is_floating_point<T>::value)
    {
      erased_lvalue.set<T>(table.GetField<T>(field.offset(), field.default_real()));
    }
    else if (std::is_integral<T>::value)
    {
      erased_lvalue.set<T>(table.GetField<T>(field.offset(), field.default_integer()));
    }
    else
    {
      // TODO: this should never happen
      assert(false);
    }
  }
};


void add_flatbuffer_field(flatbuffers::FlatBufferBuilder& fbb, const reflection::Field& field, erased_lvalue& value)
{
  static fbb_AddElement_dispatcher add_element_d;

  add_element_d(fbb, field, value);
}

void read_flatbuffer_field(const flatbuffers::Table& table, const reflection::Field& field, erased_lvalue& value)
{
  static fbb_ReadElement_dispatcher read_element_d;

  read_element_d(table, field, value);
}

flatbuffers::uoffset_t serialize_flatbuffer_table(
  flatbuffers::FlatBufferBuilder& fbb, 
  const schema_descriptor& descriptor,
  const reflection::Schema& schema,
  const type_system::type_descriptor& ti, 
  erased_lvalue& value);

flatbuffers::uoffset_t serialize_flatbuffer_vector(
  flatbuffers::FlatBufferBuilder& fbb,
  const schema_descriptor& descriptor,
  const reflection::Schema& schema,
  const type_system::type_descriptor& ti,
  erased_vector& espan)
{
  const static fbb_AddElement_dispatcher add_element_d;

  if (ti.is_builtin)
  {
    if (espan._type.is<std::string>())
    {
      return fbb.CreateVectorOfStrings(reinterpret_cast<std::string*>(espan.data()), reinterpret_cast<std::string*>(espan.data()) + espan.size()).o;
    }
    else
    {
      // this is a vector of scalars which are not strings
      uint8_t* buf;
      flatbuffers::uoffset_t result = fbb.CreateUninitializedVector(espan.size(), espan._type.size, &buf);
      espan.copy_to(reinterpret_cast<void*>(buf), espan.size() * espan._type.size);

      return result;
    }
  }
  else
  {
    std::vector<flatbuffers::uoffset_t> offsets;
    offsets.reserve(espan.size());

    uint8_t* buf = reinterpret_cast<uint8_t*>(espan.data());
    static_assert(sizeof(uint8_t) == 1, "our pointer type better be a single byte");

    const size_t advance = espan._type.size;

    for (size_t i = 0; i < espan.size(); i++)
    {
      void* pi = buf + i * advance;

      // we have to work using the one-level-higher indirect type
      // (void*) vs T&, which means 
      void** magic = reinterpret_cast<void**>(pi);

      erased_lvalue elv (espan._type, ref(*magic));

      offsets.push_back(serialize_flatbuffer_table(fbb, descriptor, schema, ti, elv));
    }

    //espan.reduce<void>(f);

    fbb.StartVector(espan.size(), sizeof(flatbuffers::uoffset_t));
    for (int i = espan.size(); i > 0;)
    {
      fbb.PushElement(offsets[--i]);
    }
    return fbb.EndVector(espan.size());
  }
}

void read_flatbuffer_vector_table(
  const flatbuffers::VectorOfAny& source,
  const reflection::Schema& schema,
  const reflection::Object& element_table,
  const type_system::type_descriptor& ti,
  erased_vector& espan)
{
  
}

void read_flatbuffer_vector_builtin(
  const flatbuffers::VectorOfAny& source,
  const reflection::Schema& schema,
  const reflection::BaseType element_type,
  const type_system::type_descriptor& ti,
  erased_vector& espan)
{
  const uint8_t* begin;
  const uint8_t* end;

  // either a vector of strings or a vector of built-in scalars
    if (espan._type.is<std::string>())
    {
      std::vector<std::string> strings;

      for (auto i = 0; i < source.size(); i++)
      {
        strings.push_back(flatbuffers::GetAnyVectorElemS(&source, element_type, i));
      }

      begin = reinterpret_cast<const uint8_t*>(strings.data());
      end = reinterpret_cast<const uint8_t*>(begin + (sizeof(std::string)));
    }
    else
    {
      begin = source.Data();
      end = begin + (source.size() * espan._type.size );
    }

  if (begin != nullptr && end - begin > 0)
  {
    espan.assign_from(begin, end);
  }
}

void read_flatbuffer_vector_field(
  const flatbuffers::Table& container,
  const reflection::Schema& schema,
  const reflection::Field& field,
  const type_system::property_descriptor& pi,
  const type_system::type_descriptor& pti,
  erased_vector& espan)
{
  assert (pi.is_vector);

  const flatbuffers::VectorOfAny* maybe_vector = flatbuffers::GetFieldAnyV(container, field);
  if (maybe_vector == nullptr)
  {
    return;
  }

  const flatbuffers::VectorOfAny& vector = *maybe_vector;

  if (pti.is_builtin)
  {
    read_flatbuffer_vector_builtin(vector, schema, field.type()->base_type(), pti, espan);
  }
  else
  {
    //read_flatbuffer_vector_table()
  }
}

flatbuffers::uoffset_t serialize_flatbuffer_table(
  flatbuffers::FlatBufferBuilder& fbb, 
  const schema_descriptor& descriptor,
  const reflection::Schema& schema,
  const type_system::type_descriptor& ti,
  erased_lvalue& value)
{
  assert(!ti.is_builtin); //, "built-in type misinterpreted as table"

  std::string qname = descriptor.make_qualified_name(ti.name);
  auto maybe_object = schema.objects()->LookupByKey(qname.c_str());
  assert(maybe_object); // "table not found in schema"

  const reflection::Object& object = *maybe_object;

  std::unordered_map<std::type_index, type_number> property_types; 
  std::unordered_map<name_t, flatbuffers::uoffset_t> offsets;

  reflectable& _reflectable = value.get<reflectable>();
  reflector r { _reflectable };

  // pass1: depth-first build the offsets for complex object (non-builtins, or strings)
  std::for_each(ti.properties_begin(), ti.properties_end(), 
    [&r, &property_types, &offsets, &fbb, &descriptor, &schema, &object](auto& it)
    {
      auto& pi = it.second;
      
      const type_descriptor* maybe_ti = nullptr;
      auto cacheit = property_types.find(pi.etype.tindex);
      if (cacheit != property_types.end())
      {
        maybe_ti = &type_registry::instance().lookup_type(cacheit->second);
      }
      else
      {
        auto typeit = type_registry::instance().find_type(pi.etype.tindex);
        if (typeit != type_registry::instance().types_end())
        {
          maybe_ti = typeit._Ptr;
        }
      }
      
      assert(maybe_ti != nullptr); //, "property type was not registered property (missing builtin?)"

      const type_descriptor& pti = *maybe_ti;
      property_types[pi.etype.tindex] = pti.typeindex;

      // Filter out non-(vector, string, table)s (in other words, scalar, non-string built-ins) 
      // after caching the type lookup
      if (!pi.is_vector && pti.is_builtin && !pi.etype.is<std::string>()) { return; }

      flatbuffers::uoffset_t offset;
      if (pi.is_vector)
      {
        erased_vector espan = r.reflect_vector(pi.name);
        offset = serialize_flatbuffer_vector(fbb, descriptor, schema, pti, espan);
      }
      else if (pti.is_builtin)
      {
        // this property is a string
        erased_lvalue pvalue = r.reflect_scalar(pi.name);
        std::string str = pvalue.get<std::string>();
        offset = fbb.CreateString(str).o;
      }
      else
      {
        // this property is a table
        erased_lvalue pvalue = r.reflect_scalar(pi.name);
        //auto inner_qname = schema_manager.make_qualified_name(pti.name);
        offset = serialize_flatbuffer_table(fbb, descriptor, schema, pti, pvalue);
      }

      offsets[pi.name] = offset;
    });

  // pass2: add the offsets and scalars to the table
  flatbuffers::uoffset_t table_offset = fbb.StartTable();

  std::for_each(ti.properties_begin(), ti.properties_end(), 
    [&r, &property_types, &offsets, &fbb, &schema, &object](auto& it)
    {
      auto& pi = it.second;
      
      // our second pass, means we know that the type is cached
      const type_descriptor& pti = type_registry::instance().lookup_type(property_types[pi.etype.tindex]);

      const reflection::Field* maybe_field = object.fields()->LookupByKey(pi.name);
      if (maybe_field == nullptr)
      {
        // this property is not in the schema
        // TODO: error?
        return;
      }

      const reflection::Field& field = *maybe_field;

      auto offsetit = offsets.find(pi.name);
      if (offsetit == offsets.end())
      {
        assert(pti.is_builtin);
        assert(!pi.is_vector); //, "builtin property cannot be a vector"
        erased_lvalue pvalue = r.reflect_scalar(pi.name);
        add_flatbuffer_field(fbb, field, pvalue);
      }
      else
      {
        fbb.AddOffset(field.offset(), flatbuffers::Offset<void>(offsetit->second));
      }
    });

  table_offset = fbb.EndTable(table_offset);
  return table_offset;
}

void read_flatbuffer_table(
    const flatbuffers::Table& source,
    const reflection::Schema& schema,
    const reflection::Object& table,
    const type_descriptor& ti,
    erased_lvalue& target)
  {
    reflectable& _reflectable = target.get<reflectable>();
    reflector r { _reflectable };

    std::unordered_map<std::type_index, type_number> property_types; 

    std::for_each(ti.properties_begin(), ti.properties_end(), 
      [&r, &property_types, &schema, &table, &source](auto& it)
      {
        auto& pi = it.second;

        // cache the type lookup
        const type_descriptor* maybe_ti = nullptr;
        auto cacheit = property_types.find(pi.etype.tindex);
        if (cacheit != property_types.end())
        {
          maybe_ti = &type_registry::instance().lookup_type(cacheit->second);
        }
        else
        {
          auto typeit = type_registry::instance().find_type(pi.etype.tindex);
          if (typeit != type_registry::instance().types_end())
          {
            maybe_ti = typeit._Ptr;
          }
        }
        
        assert(maybe_ti != nullptr); //, "property type was not registered property (missing builtin?)"

        const type_descriptor& pti = *maybe_ti;
        property_types[pi.etype.tindex] = pti.typeindex;

        //

        const reflection::Field* maybe_field = table.fields()->LookupByKey(pi.name);
        if (maybe_field == nullptr)
        {
          // this property is not in the schema
          // TODO: Error
          return;
        }

        const reflection::Field& field = *maybe_field;

        if (pi.is_vector)
        {
          flatbuffers::VectorOfAny* maybe_vector = flatbuffers::GetFieldAnyV(source, field);
          if (maybe_vector == nullptr)
          {
            // TODO: error?
            return;
          }

          erased_vector espan = r.reflect_vector(pi.name);

          read_flatbuffer_vector_field(source, schema, field, pi, pti, espan);
        }
        else if (pti.is_builtin)
        {
          erased_lvalue pvalue = r.reflect_scalar(pi.name);

          if (pi.etype.is<std::string>())
          {
            auto str = flatbuffers::GetFieldS(source, field);
            pvalue.set<std::string>(std::string(flatbuffers::GetString(str)));
          }
          else
          {
            read_flatbuffer_field(source, field, pvalue);
          }
        }
        else
        {
          // table type
          assert(field.type()->base_type() == reflection::BaseType::Obj);

          erased_lvalue pvalue = r.reflect_scalar(pi.name);
          
          // get the underlying Object* representing the table info
          flatbuffers::Table* inner_source = flatbuffers::GetFieldT(source, field);
          const reflection::Object* inner_table = schema.objects()->Get(field.type()->index());
          read_flatbuffer_table(*inner_source, schema, *inner_table, pti, pvalue);
        }
      });
  }

offset_of_any serializer::write_flatbuffer(flatbuffers::FlatBufferBuilder& fbb, erased_lvalue& target)
  {
    // todo: it would be nice to enforce these known invariants at compile time
    // but for now, we'll just know they hold and assert

    auto it = type_registry::instance().find_type(target._type.tindex);
    assert(it != type_registry::instance().types_end()); //, "type not registered");

    const type_descriptor& ti = *it;
    assert(!ti.is_builtin); //, "we cannot serialize built-ins directly; need to be in a table"

    auto maybe_schema = _schema.get();
    assert(maybe_schema); //, "schema not found for type - TODO: this is a real possible error, handle it");

    const reflection::Schema& schema = *maybe_schema;

    //print all of the names of the types in the schemas
    auto types = schema.objects();
    for ( size_t i = 0; i < types->Length(); i++ ) {
        const reflection::Object* type = types->Get( i );
        std::cout << "type: " << type->name()->str() << std::endl;
    }

    return offset_of_any{serialize_flatbuffer_table(fbb, _schema.descriptor, schema, ti, target)};
  }

  activation serializer::read_flatbuffer(const uint8_t* buf, erased_type target_type)
  {
    activation result = target_type.activator();
    reflector r{result.get<reflectable>()};

    auto it = type_registry::instance().find_type(target_type.tindex);
    assert(it != type_registry::instance().types_end()); //, "type not registered");

    const type_descriptor& ti = *it;

    auto maybe_schema = _schema.get();
    assert(maybe_schema); //, "schema not found for type - TODO: this is a real possible error, handle it");

    const reflection::Schema& schema = *maybe_schema;

    std::string qname = _schema.descriptor.make_qualified_name(ti.name);
    auto maybe_table = schema.objects()->LookupByKey(qname.c_str());
    assert(maybe_table); //, "type not found in schema");

    const reflection::Object& table = *maybe_table;

    const flatbuffers::Table* maybe_source = flatbuffers::GetAnyRoot(buf);
    assert(maybe_source); //, "buffer is not a table");

    const flatbuffers::Table& source = *maybe_source;

    read_flatbuffer_table(source, schema, table, ti, r.reflect_scalar("this"));

    return result;
  }
}