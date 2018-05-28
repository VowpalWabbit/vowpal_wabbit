#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <unordered_map>
#include "model_mgmt.h"
#include "object_factory.h"
#include "factory_resolver.h"
#include "constants.h"
#include "api_status.h"
#include "err_constants.h"

namespace r = reinforcement_learning;
namespace m = reinforcement_learning::model_mangement;
namespace u =  reinforcement_learning::utility;

const char * A_BLOB_URI   = "https://storagex6ilrr3obwyra.blob.core.windows.net/mwt-models/current?sv=2017-04-17&sr=b&sig=n9udcFiyjhahgnK38elMIEbg9FYWfmmAWte1MP87v1s%3D&st=2018-03-19T17%3A51%3A18Z&se=2028-03-19T17%3A52%3A18Z&sp=r";
const char * CFG_VALUE = "model.vw";

m::model_data get_model_data();

BOOST_AUTO_TEST_CASE(factory_tempate_usage)
{
  struct an_interface { virtual void do_something() = 0; virtual ~an_interface(){} };
  struct impl_A : public an_interface { void do_something() override {} };
  struct impl_B : public an_interface { explicit impl_B(int b) {} void do_something() override {} };

  auto b = 5;  // arbitrary variable to illustrate a point
  u::object_factory<an_interface> factory;

  auto create_A_fn = [](an_interface** pret, const u::config_collection&, r::api_status*) -> int 
  { 
    *pret = new impl_A();
    return r::error_code::success;
  };

  auto create_B_fn = [b](an_interface** pret, const u::config_collection&, r::api_status*) -> int 
  { 
    *pret = new impl_B(b); 
    return r::error_code::success;
  };

  factory.register_type("A", create_A_fn);
  factory.register_type("B", create_B_fn);

  u::config_collection cc;
  cc.set(r::name::MODEL_SRC, r::value::AZURE_STORAGE_BLOB);

  an_interface* p_impl;
  auto scode = factory.create(&p_impl,std::string("A"),cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  p_impl->do_something();
  delete p_impl;
}

BOOST_AUTO_TEST_CASE(azure_storage_model_data)
{
  u::config_collection cc;
  cc.set(r::name::MODEL_BLOB_URI,  A_BLOB_URI);

  m::i_data_transport* data_transport;
  auto scode = r::data_transport_factory.create(&data_transport, r::value::AZURE_STORAGE_BLOB, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  m::model_data md;
  scode = data_transport->get_data(md);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  delete data_transport;
}

void register_local_file_factory();
r::str_const DUMMY_DATA_TRANSPORT = "DUMMY_DATA_TRANSPORT";
r::str_const CFG_PARAM = "model.local.file";

BOOST_AUTO_TEST_CASE(data_transport_user_extention)
{
  register_local_file_factory();

  u::config_collection cc;
  cc.set(CFG_PARAM,  CFG_VALUE);

  m::i_data_transport* data_transport;
  auto scode = r::data_transport_factory.create(&data_transport, DUMMY_DATA_TRANSPORT, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  m::model_data md;
  scode = data_transport->get_data(md);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  delete data_transport;
}

BOOST_AUTO_TEST_CASE(vw_model_factory)
{
  auto model = get_model_data();

  u::config_collection model_cc;
  model_cc.set(r::name::VW_CMDLINE, "--lda 5");
  m::i_model* vw;
  auto scode = r::model_factory.create(&vw, r::value::VW, model_cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  vw->init(model);

  char* features = "1 2 3";
  int actions[] = { 1,2,3 };
  int action;
  scode = vw->choose_rank(action, features, actions);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  delete vw;
}

m::model_data get_model_data()
{
  u::config_collection cc;
  cc.set(CFG_PARAM, CFG_VALUE);

  m::i_data_transport* data_transport;
  r::data_transport_factory.create(&data_transport, DUMMY_DATA_TRANSPORT, cc);
  std::unique_ptr<m::i_data_transport> pdt(data_transport);
  m::model_data md;
  auto scode = pdt->get_data(md);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  return md;
}

class dummy_data_transport : public m::i_data_transport {
  virtual int get_data(m::model_data& data, r::api_status* status = nullptr) {
    data.data = nullptr;
    data.data_sz = 0;
    return r::error_code::success;
  }
};

int dummy_data_tranport_create( m::i_data_transport** retval, 
                                    const u::config_collection& cfg, 
                                    r::api_status* status) {
  *retval = new dummy_data_transport();
  return r::error_code::success;
}

void register_local_file_factory() {
  r::data_transport_factory.register_type(DUMMY_DATA_TRANSPORT, dummy_data_tranport_create);
}
