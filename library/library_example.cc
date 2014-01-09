#include <stdio.h>
#include "../vowpalwabbit/parser.h"
#include "../vowpalwabbit/vw.h"

using namespace std;


inline feature vw_feature_from_string(vw& v, string fstr, unsigned long seed, float val)
{
  uint32_t foo = VW::hash_feature(v, fstr, seed);
  feature f = { val, foo};
  return f;
}

int main(int argc, char *argv[])
{
  vw* model = VW::initialize("--hash all -q st --noconstant -i train.w -f train2.vw");

  example *vec2 = VW::read_example(*model, (char*)"|s p^the_man w^the w^man |t p^un_homme w^un w^homme");
  model->learn(vec2);
  cerr << "p2 = " << vec2->final_prediction << endl;
  VW::finish_example(*model, vec2);

  vector< VW::feature_space > ec_info;
  vector<feature> s_features, t_features;
  uint32_t s_hash = VW::hash_space(*model, "s");
  uint32_t t_hash = VW::hash_space(*model, "t");
  s_features.push_back( vw_feature_from_string(*model, "p^the_man", s_hash, 1.0) );
  s_features.push_back( vw_feature_from_string(*model, "w^the", s_hash, 1.0) );
  s_features.push_back( vw_feature_from_string(*model, "w^man", s_hash, 1.0) );
  t_features.push_back( vw_feature_from_string(*model, "p^le_homme", t_hash, 1.0) );
  t_features.push_back( vw_feature_from_string(*model, "w^le", t_hash, 1.0) );
  t_features.push_back( vw_feature_from_string(*model, "w^homme", t_hash, 1.0) );
  ec_info.push_back( VW::feature_space('s', s_features) );
  ec_info.push_back( VW::feature_space('t', t_features) );
  example* vec3 = VW::import_example(*model, ec_info);
    
  model->learn(vec3);
  cerr << "p3 = " << vec3->final_prediction << endl;
  VW::finish_example(*model, vec3);

  VW::finish(*model);

  vw* model2 = VW::initialize("--hash all -q st --noconstant -i train2.vw");
  vec2 = VW::read_example(*model2, (char*)" |s p^the_man w^the w^man |t p^un_homme w^un w^homme");
  model2->learn(vec2);
  cerr << "p4 = " << vec2->final_prediction << endl;

  size_t len=0;
  VW::primitive_feature_space* pfs = VW::export_example(*model2, vec2, len);
  for (size_t i = 0; i < len; i++)
    {
      cout << "namespace = " << pfs[i].name;
      for (size_t j = 0; j < pfs[i].len; j++)
	cout << " " << pfs[i].fs[j].weight_index << ":" << pfs[i].fs[j].x << ":" << VW::get_weight(*model2, pfs[i].fs[j].weight_index, 0);
      cout << endl;
    }   

  VW::finish_example(*model2, vec2);
  VW::finish(*model2);
}

