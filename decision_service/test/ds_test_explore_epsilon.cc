/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/


#include <boost/test/unit_test.hpp>

#include "ds_explore.h"

using namespace boost::unit_test;
using namespace std;
using namespace Microsoft::DecisionService;

BOOST_AUTO_TEST_SUITE( DecisionServiceExploreEpsilon )

BOOST_AUTO_TEST_CASE( EpsilonGreedy )
{
    EpsilonGreedyExplorer explorer(0.3);

    vector<float> scores = { 1, 2, 3 };
    DecisionServicePredictorsSimple predictor(scores);
    PredictorContainer container(&predictor);
    ActionProbabilities actionProbs = explorer.explore(container);

    BOOST_CHECK_EQUAL(actionProbs.size(), 3 );

    // sorted by score
    BOOST_CHECK_EQUAL(actionProbs[0].action, 2);
    BOOST_CHECK_EQUAL(actionProbs[1].action, 1);
    BOOST_CHECK_EQUAL(actionProbs[2].action, 0);

    BOOST_CHECK_CLOSE(actionProbs[0].probability, 0.8, 0.00001);
    BOOST_CHECK_CLOSE(actionProbs[1].probability, 0.1, 0.00001);
    BOOST_CHECK_CLOSE(actionProbs[2].probability, 0.1, 0.00001);
}

BOOST_AUTO_TEST_CASE( EpsilonGreedyStable )
{
    EpsilonGreedyExplorer bag(0.3);

    vector<float> scores = { 0, 1, 1 };
    DecisionServicePredictorsSimple predictor(scores);
    PredictorContainer container(&predictor);
    ActionProbabilities actionProbs = bag.explore(container);

    BOOST_CHECK_EQUAL(actionProbs.size(), 3);

    // sorted by score
    BOOST_CHECK_EQUAL(actionProbs[0].action, 1);
    BOOST_CHECK_EQUAL(actionProbs[1].action, 2);
    BOOST_CHECK_EQUAL(actionProbs[2].action, 0);

    BOOST_CHECK_CLOSE(actionProbs[0].probability, 0.8, 0.00001);
    BOOST_CHECK_CLOSE(actionProbs[1].probability, 0.1, 0.00001);
    BOOST_CHECK_CLOSE(actionProbs[2].probability, 0.1, 0.00001);
}

BOOST_AUTO_TEST_SUITE_END()