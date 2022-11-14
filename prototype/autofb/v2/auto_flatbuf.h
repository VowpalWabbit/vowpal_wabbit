#pragma once

#include "base.h"

#include "type_registry.h"
#include "type_construtors.h"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/reflection_generated.h"

namespace auto_flatbuf
{
  using type_system::type_registry;
  using type_system::type_descriptor;
  using type_system::reflectable;
  using type_system::reflector;

  using offset_of_any = flatbuffers::Offset<void>;

  void generate_universe_types(
    const std::string& schema_ns, 
    const std::string& output_dir,
    type_registry& types = type_registry::instance()
  );

  void print_universe_types(
    const std::string& schema_ns, 
    type_registry& types = type_registry::instance()
  );

  struct schema_descriptor
  {
    std::string schema_namespace; // the default namespace

    // TODO: version and inheritance?
    // dependent schemas?

    inline std::string make_qualified_name(std::string type_name) const 
    {
      return schema_namespace + "." + type_name;
    }
  };

  using schema_loader_f = std::function<bool(const std::string& schema_name, std::string& out_buf)>;

  struct file_schema_loader
  {
    std::string bfbs_search_dir;

    bool operator()(const std::string& schema_name, std::string& out_buf)
    {
      std::string schema_path = make_schema_path(schema_name);
      if (!flatbuffers::LoadFile(schema_path.c_str(), true, &out_buf))
      {
        return false;
      }

      // check if we can read it as a schema
      const reflection::Schema* maybe_schema = reflection::GetSchema(out_buf.c_str());
      if (!maybe_schema)
      {
        return false;
      }

      return true;
    }

  private:
    inline std::string make_schema_path(const std::string& schema_name) const
    {
      return bfbs_search_dir + schema_name + ".bfbs";
    }
  };

  // class schema_manager
  // {
  // public:
  //   schema_manager(schema_descriptor descriptor, schema_loader_f loader)
  //     : descriptor{ descriptor }, loader{ loader }
  //   {
  //   }

  //   inline std::string make_qualified_name(const std::string& name) const
  //   {
  //     return descriptor.make_qualified_name(name);
  //   }

  //   inline const reflection::Schema* try_load_schema_for_type(const type_descriptor& ti)
  //   {
  //     return try_load_schema_by_name(ti.name);
  //   }
    
  //   const reflection::Schema* try_load_schema_by_name(const std::string& name)
  //   {
  //     std::string qname = make_qualified_name(name);

  //     // check if it is in the cache, otherwise load it from the search path
  //     auto it = _schema_cache.find(qname);
  //     if (it != _schema_cache.end())
  //     {
  //       return reflection::GetSchema(it->second.c_str());;
  //     }
  //     else
  //     {
  //       // TODO: How to get folder separator in OS independent way?
  //       const std::string FOLDER_SEPARATOR = "/";

  //       std::string schema_buffer;
  //       if (!loader(name, schema_buffer))
  //       {
  //         return nullptr;
  //       }

  //       _schema_cache[qname] = schema_buffer;
  //       return reflection::GetSchema(_schema_cache[qname].c_str());
  //     }
  //   }

  // private:
  //   schema_descriptor descriptor;
  //   schema_loader_f loader;

  //   using schema_cache_t = std::unordered_map<std::string, std::string>;
  //   schema_cache_t _schema_cache;
  // };

struct bfbs_data
  {
    std::string binary_data;
  };

  struct fbs_data
  {
    std::string text_data;

    bfbs_data to_binary();
  };

  // schema represents a set of well-defined flat-buffer 
  class schema
  {
  public:
    schema(schema_descriptor descriptor, bfbs_data schema_data) 
      : descriptor{ descriptor }, schema_data{ schema_data }
    {
    }

    schema(std::string default_namespace, std::string path) 
      : descriptor{ default_namespace }
    {
      if (flatbuffers::LoadFile(path.c_str(), true, &schema_data.binary_data))
      {
      }
      else
      {
        // TODO: Error out?
      }
    }

    const schema_descriptor descriptor;

    const reflection::Schema* get() const
    {
      return reflection::GetSchema(schema_data.binary_data.c_str());
    }

  private:
    bfbs_data schema_data;
  };

  class serializer
{
public:
  serializer(schema schema) : 
    _schema{schema}
  {
  }

  inline offset_of_any write_flatbuffer(flatbuffers::FlatBufferBuilder& fbb, reflectable& target)
  {
    reflector r{target};
    erased_lvalue elv_target = r.reflect_scalar("this");

    return write_flatbuffer(fbb, elv_target);
  }

  offset_of_any write_flatbuffer(flatbuffers::FlatBufferBuilder& fbb, erased_lvalue& target);

  activation read_flatbuffer(const uint8_t* buf, erased_type target_type);

private:
  schema _schema;
};

  // this has slightly different semantics than generate/print_universe_types. the intent here is to
  // take in a universe and a namespace for it, and generate a single schema for that namespace, 
  // containing all the types.
  class schema_builder
  {
  public:
    schema_builder(std::string ns, const type_registry& types = type_registry::instance())
      : descriptor{ ns }, types{ types }
    {
      // should we do the build-out on construction?
    }

    fbs_data build_idl();

    inline schema build()
    {
      return schema{ descriptor, build_idl().to_binary() };
    }

  private:
    schema_descriptor descriptor;
    const type_registry& types;
  };
}