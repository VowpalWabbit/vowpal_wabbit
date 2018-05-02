#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <unordered_map>
#include "model_mgmt/ds_model_mgmt.h"
#include "utility/ds_object_factory.h"

using namespace decision_service::model_mangement;
using namespace decision_service::utility;

const char * A_BLOB_URI   = "https://storagex6ilrr3obwyra.blob.core.windows.net/mwt-models/current?sv=2017-04-17&sr=b&sig=n9udcFiyjhahgnK38elMIEbg9FYWfmmAWte1MP87v1s%3D&st=2018-03-19T17%3A51%3A18Z&se=2028-03-19T17%3A52%3A18Z&sp=r";
const char * A_LOCAL_FILE = "model.vw";

model_data get_model_data();

BOOST_AUTO_TEST_CASE(factory_tempate_usage)
{
  struct an_interface { virtual void do_something() = 0; virtual ~an_interface(){} };
  struct impl_A : public an_interface { void do_something() override {} };
  struct impl_B : public an_interface { explicit impl_B(int b) {} void do_something() override {} };

  auto b = 5;  // arbitrary variable to illustrate a point
  object_factory<an_interface> factory;

  auto create_A_fn = [](const config_collection&) -> an_interface* { return new impl_A(); };
  auto create_B_fn = [b](const config_collection&) -> an_interface* { return new impl_B(b); };

  factory.register_type("A", create_A_fn);
  factory.register_type("B", create_B_fn);

  config_collection cc;
  cc.set(name::MODEL_SRC, value::AZURE_STORAGE_BLOB);

  auto p_impl = factory.create(std::string("A"),cc);
  p_impl->do_something();
}

BOOST_AUTO_TEST_CASE(azure_storage_model_data)
{
  config_collection cc;
  cc.set(name::MODEL_BLOB_URI,  A_BLOB_URI);

  auto data_transport = model_data_factory().create(value::AZURE_STORAGE_BLOB, cc);
  auto model_data = data_transport->get_data();
}

BOOST_AUTO_TEST_CASE(local_file_model_data)
{
  config_collection cc;
  cc.set(name::MODEL_LOCAL_FILE,  A_LOCAL_FILE);

  auto data_transport = model_data_factory().create(value::LOCAL_FILE, cc);
  auto model_data = data_transport->get_data();
}

BOOST_AUTO_TEST_CASE(vw_model_factory)
{
  auto model = get_model_data();

  config_collection model_cc;
  model_cc.set(name::VW_CMDLINE, "--lda 5");
  auto vw = model_factory().create(value::VW,model_cc);
  vw->init(model);

  char* features = "1 2 3";
  int actions[] = { 1,2,3 };
  int action = vw->choose(features, actions);
}

model_data get_model_data()
{
  config_collection cc;
  cc.set(name::MODEL_LOCAL_FILE, A_LOCAL_FILE);

  auto data_transport = model_data_factory().create(value::LOCAL_FILE, cc);
  return data_transport->get_data();
}
