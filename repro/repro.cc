#include "vw.h"

int main() {
  auto m_vw = VW::initialize("--cb_explore_adf --epsilon 0 -l 700 --power_t 0", nullptr, false, nullptr, nullptr);
  multi_ex ex_coll;
  m_vw->learn(ex_coll);
}
