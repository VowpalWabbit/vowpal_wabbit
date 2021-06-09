#pragma once

#include "global_data.h" // to get vw struct
#include "options.h" // to get options_i

#include "gd.h"
#include "scorer.h"


namespace test_mctest
{

// ugly
VW::LEARNER::base_learner* prev = nullptr;
VW::LEARNER::base_learner* return_prev(VW::config::options_i&, vw&)
{
    return prev;
}

VW::LEARNER::base_learner* nullptr_base_learner_creator(VW::config::options_i&, vw&)
{
    return nullptr;
}

void instantiate_learner(VW::config::options_i& options, vw& all)
{
    VW::LEARNER::base_learner* gd_learner = GD::setup(nullptr_base_learner_creator, options, all);

    // set prev for return_prev fn
    prev = gd_learner;

    VW::LEARNER::base_learner* scorer = scorer_setup(return_prev, options, all);

    // be good citizen and update enabled reductions
    all.enabled_reductions.push_back("gd");
    all.enabled_reductions.push_back("scorer");

    // set to top level reduction
    all.l = scorer;

    return;
}
}