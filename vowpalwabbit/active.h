#pragma once

struct active {

	// active learning algorithm parameters
	float active_c0;
	vw* all;//statistics, loss
	bool oracular;
};

float query_decision(active& a, example& ec, float k);
LEARNER::base_learner* active_setup(vw& all);
LEARNER::base_learner* active_oracular_setup(vw& all);
