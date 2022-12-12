#include "vwtypes.h"

namespace pseudo_vw
{
VW::LEARNER::base_learner* stack_builder_t::setup_base_learner() { return nullptr; };
VW::config::options_i* stack_builder_t::get_options() { return nullptr; }
magic_t* stack_builder_t::get_all_pointer() { return nullptr; }
}  // namespace pseudo_vw
