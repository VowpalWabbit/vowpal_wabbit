#include "vw/config/options_cli.h"
#include "vw/core/parser.h"
#include "vw/core/vw.h"

#include <cstdio>

inline VW::feature vw_feature_from_string(VW::workspace& v, const std::string& fstr, unsigned long seed, float val)
{
  auto foo = VW::hash_feature(v, fstr, seed);
  VW::feature f = {val, foo};
  return f;
}

int main(int argc, char* argv[])
{
  auto model = VW::initialize(VW::make_unique<VW::config::options_cli>(
      std::vector<std::string>{"--hash", "all", "-q", "st", "--noconstant", "-f", "train2.vw", "--no_stdin"}));

  VW::example* vec2 = VW::read_example(*model, (char*)"|s p^the_man w^the w^man |t p^un_homme w^un w^homme");
  model->learn(*vec2);
  std::cerr << "p2 = " << vec2->pred.scalar << std::endl;
  VW::finish_example(*model, *vec2);

  VW::primitive_feature_space features[2];
  VW::primitive_feature_space *s = features, *t = features + 1;
  s->name = 's';
  t->name = 't';

  uint32_t s_hash = static_cast<uint32_t>(VW::hash_space(*model, "s"));
  uint32_t t_hash = static_cast<uint32_t>(VW::hash_space(*model, "t"));
  s->fs = new VW::feature[3];
  s->len = 3;
  t->fs = new VW::feature[3];
  t->len = 3;

  s->fs[0] = vw_feature_from_string(*model, "p^the_man", s_hash, 1.0);
  s->fs[1] = vw_feature_from_string(*model, "w^the", s_hash, 1.0);
  s->fs[2] = vw_feature_from_string(*model, "w^man", s_hash, 1.0);
  t->fs[0] = vw_feature_from_string(*model, "p^le_homme", t_hash, 1.0);
  t->fs[1] = vw_feature_from_string(*model, "w^le", t_hash, 1.0);
  t->fs[2] = vw_feature_from_string(*model, "w^homme", t_hash, 1.0);
  VW::example* vec3 = VW::import_example(*model, "", features, 2);

  model->learn(*vec3);
  std::cerr << "p3 = " << vec3->pred.scalar << std::endl;
  // TODO: this does not invoke m_vw->l->finish_example()
  VW::finish_example(*model, *vec3);
  model->finish();
  model.reset();

  auto model2 = VW::initialize(VW::make_unique<VW::config::options_cli>(
      std::vector<std::string>{"--hash", "all", "-q", "st", "--noconstant", "-i", "train2.vw", "--no_stdin"}));
  vec2 = VW::read_example(*model2, (char*)" |s p^the_man w^the w^man |t p^un_homme w^un w^homme");
  model2->learn(*vec2);
  std::cerr << "p4 = " << vec2->pred.scalar << std::endl;

  size_t len = 0;
  VW::primitive_feature_space* pfs = VW::export_example(*model2, vec2, len);
  for (size_t i = 0; i < len; i++)
  {
    std::cout << "namespace = " << pfs[i].name;
    for (size_t j = 0; j < pfs[i].len; j++)
    {
      std::cout << " " << pfs[i].fs[j].weight_index << ":" << pfs[i].fs[j].x << ":"
                << VW::get_weight(*model2, static_cast<uint32_t>(pfs[i].fs[j].weight_index), 0);
    }
    std::cout << std::endl;
  }

  VW::finish_example(*model2, *vec2);
  model2->finish();
  model2.reset();
}
