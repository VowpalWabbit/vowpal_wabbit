#include "test.h"

#include "type_registry.h"
#include "auto_flatbuf.h"
#include "type_construtors.h"

#include "flatbuffers\flatbuffers.h"
#include "flatbuffers\idl.h"
#include "flatbuffers\reflection.h"
#include "flatbuffers\reflection_generated.h"

using std::cout;
using std::endl;

using namespace type_system;

struct manual_test1
{
  float x;
  std::vector<float> y;
};

struct manual_test2
{
  manual_test1 t1;
};

template <const type_registration_evidence& registration>
struct type_helper
{
public:
  static const type_descriptor& descriptor()
  {
    return registration.descriptor;
  }
};

void do_test_manual()
{
  cout << "Printing the universe types!" << endl;

  type_registry& registry = type_registry::instance();
  auto& test1d = registry.register_type("manual_test1", type<manual_test1>::erase());
  test1d.set_activator(&default_activator<manual_test1>::activate);
  
  test1d.define_property("x", type<float>::erase(), false);
  test1d.define_property("y", type<float>::erase(), true);

  auto& test2d = registry.register_type("manual_test2", type<manual_test2>::erase());
  test2d.set_activator(&default_activator<manual_test2>::activate);

  test2d.define_property("t1", type<manual_test1>::erase(), false);

  auto_flatbuf::print_universe_types("autofb");
}

TC_DATA(test1)
{ 
  private: 
    inline static property_descriptor& __register_x() 
    { return register_property("x", type<float>::erase(), false); }; 
            
    inline static staticinit::witness<property_descriptor&, &__register_x> __witnessx() 
    { static staticinit::witness<property_descriptor&, &__register_x> w; return w; }; 
  public: 
    property_<float> x = { reflection_beacon, __witnessx().get_evidence() };

  v_(float, y);
};

TC_DATA(test3)
{
  v_(test1, t1);
  v_(std::string, t2);
  _(std::string, t3);

  DERIVE_REFLECT(test3);
};

TC_DATA(simple_types)
{
  #define __(type) _(type, _ ## type)
  __(bool);
  __(int8_t);
  __(uint8_t);
  __(int16_t);
  __(uint16_t);
  __(int32_t);
  __(uint32_t);
  __(int64_t);
  __(uint64_t);
  __(float);
  __(double);
  #undef __
};

TC_DATA(byte_vector)
{
  v_(uint8_t, u8v);
};

using namespace auto_flatbuf;

test1 make_test1(float x, std::initializer_list<float> ys)
{
  test1 t;
  t.x = x;
  t.y.assign(ys.begin(), ys.end());
  return t;
}

test3 make_test3(std::initializer_list<test1> t1s,
                 std::initializer_list<std::string> t2s,
                 std::string t3)
{
  test3 t;
  t.t1.assign(t1s.begin(), t1s.end());
  //t.t2.assign(t2s.begin(), t2s.end());
  t.t3 = t3;
  return t;
}

const std::string schema_dir = "C:\\s\\vw\\vw_proto\\prototype\\autofb\\schemas\\"; 
const std::string gen_dir = "C:\\s\\vw\\vw_proto\\prototype\\autofb\\generated\\";

static auto_flatbuf::schema_descriptor schema_descriptor{"VW_proto"};

void try_reflect_built_schema()
{
  auto_flatbuf::schema_builder builder { "VW_proto" };

  
  // fbs_data test1_fbs = builder.build_idl();
  // std::cout << test1_fbs.text_data << std::endl;

  // flatbuffers::IDLOptions opts;
  // //opts.size_prefixed = true;

  // std::string bfbs_data_str;
  // {  
  //   flatbuffers::Parser parser;
  //   if (!parser.Parse(test1_fbs.text_data.c_str()))
  //   {
  //     // TODO: ERROR!
  //     std::string last_error = "Failed to parse schema: " + parser.error_;
  //     std::cout << last_error << std::endl << std::endl;
  //     std::cout << test1_fbs.text_data << std::endl;

  //     throw new std::exception(last_error.c_str());
  //   }

  //   parser.Serialize();

  //   auto bfbs_buffer = (char*)parser.builder_.GetBufferPointer();
  //   auto bfbs_length = parser.builder_.GetSize();

  //   bfbs_data_str.resize(bfbs_length);
  //   char* begin = &(bfbs_data_str[0]);
  //   memcpy(begin, bfbs_buffer, bfbs_length);
  // }

  //flatbuffers::DetachedBuffer buf = parser.builder_.Release();
  //std::string bfbs_data_str;
  //bfbs_data_str.assign((char*)bfbs_buffer, bfbs_length);

  //const reflection::Schema* maybe_schema = reflection::GetSchema(bfbs_data_str.c_str());
  schema schema = builder.build();
  const reflection::Schema* maybe_fbschema = schema.get();
  if (maybe_fbschema != nullptr)
  {
    const reflection::Schema& fbschema = *maybe_fbschema;
    auto objects = fbschema.objects();

    for (auto it = objects->begin(); it != objects->end(); ++it)
    {
      const reflection::Object* object = *it;
      std::cout << "Object: " << object->name()->str() << std::endl;
    }
  }

  // The assertion here is simply to flag a situation in which the fb API changes somehow so that its
  // buffer representation is no longer conveniently put into a std::string. (We should probably be
  // using vectors instead, honestly, but this is based on the reflection examples.)
  // static_assert(sizeof(std::remove_pointer_t<decltype(bfbs_buffer)>) == sizeof(char), 
  //     "fb buffer type is not the same as char*");
  
  // iterate all the objects on rb_schema
  //auto schema_objects = rb_schema.objects();
}

#include "../generated/test1_3_generated.h"

void try_reflect()
{
  //auto_flatbuf::schema_descriptor descriptor { "VW_proto" };
  auto_flatbuf::schema_builder builder { "VW_proto" };
  auto_flatbuf::schema schema = builder.build();

  serializer serializer(schema);

  test3 t = make_test3({ make_test1(1.0f, { 1.0f, 2.0f, 3.0f }),
                         make_test1(2.0f, { 4.0f, 5.0f, 6.0f }) },
                       { "hello", "world" },
                       "t3");

  flatbuffers::FlatBufferBuilder fbb;
  
  offset_of_any offset = serializer.write_flatbuffer(fbb, t.as_reflectable());

  fbb.Finish(flatbuffers::Offset<void>(offset));

  uint8_t* buf = fbb.GetBufferPointer();
  size_t size = fbb.GetSize();

  auto test3a = VW_proto::Gettest3a(buf)->UnPack();

  std::string testschema;
  std::string bfbspath = (gen_dir + "test1_3.bfbs");
  flatbuffers::LoadFile(bfbspath.c_str(), true, &testschema);


  //auto test3_table = VW_proto::Createtest3(buf);

  flatbuffers::Parser parser;

  auto idl = builder.build_idl();

  const reflection::Schema* maybe_schema = reflection::GetSchema(testschema.c_str());
  parser.Deserialize(maybe_schema);
  //parser.Parse(idl.text_data.c_str());
  //parser.Deserialize(schema.get());
  //parser.SetRootType("test3");

  std::string fb_text;
  flatbuffers::GenerateText(parser, buf, &fb_text);
  std::cout << fb_text << std::endl;

  erased_type etype = type<test3>::erase();
  //activation read_target = etype.activator();

  activation read_target = serializer.read_flatbuffer(buf, etype);

  // cheat, by pulling out the type
  test3& t1_read = read_target.get<test3>();

  // hardcoded loading for now
  // const test2::test1* t1back = flatbuffers::GetRoot<test2::test1>(buf);
  // if (t1back == nullptr)
  // {
  //   std::cout << "error loading" << std::endl;
  //   return;
  // }
  // test& t1_read = *t1back;

  std::cout << "x: " << t1_read.t3 << std::endl;
  std::cout << "y[" << t1_read.t1.size() <<"].x: ";

  //std::cout << "y: " << t1back->y()->Get(0) << std::endl;
  for (const test1& t1r_t1 : t1_read.t1)
  {
    std::cout << "  " << t1r_t1.x << std::endl;
  }
}

void test_beacon()
{
  using function_t = std::function<void*()>;

  struct callback_holder
  {
    void add_callback(function_t f)
    {
      std::cout << "adding callback@" << (void*)this << std::endl;
      callbacks.push_back(f);
    }

    // call callback in inverse order
    void call_callbacks()
    {
      for (auto it = callbacks.rbegin(); it != callbacks.rend(); ++it)
      {
        (*it)();
      }
    }

    std::vector<function_t> callbacks;
  };

  // the goal of this object is to help contained objects keep track of their container pointer
  // while moving around
  struct base_container
  {
    base_container() : registration_beacon{&callbacks}
    { 
      std::cout << "base_container()@" << (void*)this << std::endl; 
    }

    protected:
      beacon<callback_holder> registration_beacon;

  public:
    void invoke_callback()
    {
      callbacks.call_callbacks();
    }

    private:
      callback_holder callbacks;
  };

  struct contained
  {
    contained(const beacon<callback_holder>& beacon) : cobeacon{beacon.unchase<struct contained>(this)}
    {
      register_callback();

      std::cout << "contained()@" << (void*)this << std::endl; 
    }

    contained(const contained& other) : cobeacon{other.cobeacon}
    {
      register_callback();

      std::cout << "contained(const contained&)@" << (void*)this << std::endl; 
    }

    void register_callback()
    {
      cobeacon.chase(this).get().add_callback(make_callback());
    }

    function_t make_callback()
    {
      return [this]() 
      {
        std::cout << "contained::make_callback_lambda()@" << (void*)this << std::endl; 
        return this; 
      };
    }

    bool b;
    beacon<callback_holder>::cobeacon<struct contained> cobeacon;
  };

  struct container : base_container
  {
    container() 
    { 
      std::cout << "container()@" << (void*)this << std::endl; 
      register_with_base();
    }

    // container(const container& other)
    // {
    //   std::cout << "container(const container&)@" << (void*)this << std::endl;
    //   register_with_base();
    // }

    container(container&& other)
    {
      std::cout << "container(container&&)@" << (void*)this << std::endl;
      register_with_base();
    }

    private:
      function_t make_container_callback()
      {
        return [this]() 
        {
          std::cout << "container::make_container_callback_lambda()@" << (void*)this << std::endl; 
          return this; 
        };
      }

      void register_with_base()
      {
        std::cout << "container::register_with_base()@" << (void*)this << std::endl; 
        registration_beacon.get().add_callback(make_container_callback());
      }

    protected:
      const beacon<callback_holder>& get_beacon() // this is needed to fix diamond inheritance
      { 
        std::cout << "container::get_beacon()@" << (void*)this << std::endl; 
        return registration_beacon;
      }
  };

  struct D : container
  {
    contained c = contained(get_beacon());
  };

  struct D_factory
  {
    static D make_D()
    {
      D d;
      d.c.b = true;
      return d;
    };
  };

  struct test_UP
  {
    std::unique_ptr<bool> make()
    {
      std::unique_ptr<bool> p = std::make_unique<bool>(true);
      p = false;
      return p;
    };
  };

  auto p = test_UP().make();

  D d = D_factory::make_D();

  std::cout << "d.c@" << (void*)&d.c << std::endl;

  d.invoke_callback();

  //std::cout << "bool is_arithmetic: " << std::is_arithmetic<bool>::value << std::endl;
}

void do_test_sandbox();
void do_test()
{
  //auto_flatbuf::generate_universe_types(::schema_descriptor.schema_namespace, schema_dir);

  // TODO: Parse the schema files and generate the binary schemas at runtime?

  //try_reflect_built_schema();
  try_reflect();
  //do_test_sandbox();

  //test_kernel_svm2();
}

void do_test_sandbox()
{

}
