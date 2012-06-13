#include <stdio.h>
#include "../vowpalwabbit/vw.h"
#include "ezexample.h"

using namespace std;

inline feature vw_feature_from_string(vw& v, string fstr, unsigned long seed, float val)
{
  uint32_t foo = VW::hash_feature(v, fstr, seed);
  feature f = { val, foo};
  return f;
}

int main(int argc, char *argv[])
{
  // INITIALIZE WITH WHATEVER YOU WOULD PUT ON THE VW COMMAND LINE -- THIS READS IN A MODEL FROM train.w
  vw vw = VW::initialize("--hash all -q st --noconstant -i train.w -t --quiet");

  // HAL'S SPIFFY INTERFACE USING C++ CRAZINESS
  ezexample ex(&vw, false);
  ex(vw_namespace('s'))
    ("p^the_man")
    ("w^the")
    ("w^man")
    (vw_namespace('t'))
    ("p^le_homme")
    ("w^le")
    ("w^homme");
  cerr << "p_good = " << ex() << endl;

  --ex;   // remove the most recent namespace
  ex(vw_namespace('t'))
    ("p^un_homme")
    ("w^un")
    ("w^homme");
  cerr << "p_bad = " << ex() << endl;

  /*
  // JOHN'S CLUNKY INTERFACE USING STRINGS
  example *vec1 = VW::read_example(vw, (char*)"|s p^the_man w^the w^man |t p^un_homme w^un w^homme");
  vw.learn(&vw, vec1);
  cerr << "p1 = " << vec1->final_prediction << endl;
  VW::finish_example(vw, vec1);

  example *vec2 = VW::read_example(vw, (char*)"|s p^the_man w^the w^man |t p^le_homme w^le w^homme");
  vw.learn(&vw, vec2);
  cerr << "p2 = " << vec2->final_prediction << endl;
  VW::finish_example(vw, vec2);

  // JOHN'S CLUNKY INTERFACE USING VECTORS
  vector< VW::feature_space > ec_info;
  vector<feature> s_features, t_features;
  uint32_t s_hash = VW::hash_space(vw, "s");
  uint32_t t_hash = VW::hash_space(vw, "t");
  s_features.push_back( vw_feature_from_string(vw, "p^the_man", s_hash, 1.0) );
  s_features.push_back( vw_feature_from_string(vw, "w^the", s_hash, 1.0) );
  s_features.push_back( vw_feature_from_string(vw, "w^man", s_hash, 1.0) );
  t_features.push_back( vw_feature_from_string(vw, "p^le_homme", t_hash, 1.0) );
  t_features.push_back( vw_feature_from_string(vw, "w^le", t_hash, 1.0) );
  t_features.push_back( vw_feature_from_string(vw, "w^homme", t_hash, 1.0) );
  ec_info.push_back( VW::feature_space('s', s_features) );
  ec_info.push_back( VW::feature_space('t', t_features) );
  example* vec3 = VW::import_example(vw, ec_info);
  vw.learn(&vw, vec3);
  cerr << "p3 = " << vec3->final_prediction << endl;
  VW::finish_example(vw, vec3);
*/

  // AND FINISH UP
  vw.finish(&vw);
}
