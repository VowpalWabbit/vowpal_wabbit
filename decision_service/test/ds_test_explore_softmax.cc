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

BOOST_AUTO_TEST_SUITE( DecisionServiceExploreSoftmax )

BOOST_AUTO_TEST_CASE( Softmax )
{
    SoftmaxExplorer explorer(1, 0);

    vector<float> scores = { 1, 2, 3 };
    DecisionServicePredictorsSimple predictor(scores);
    PredictorContainer container(&predictor);
    ActionProbabilities actionProbs = explorer.explore(container);

    BOOST_CHECK_EQUAL(actionProbs.size(), 3 );

    // sorted by score
    BOOST_CHECK_EQUAL(actionProbs[0].action, 2);
    BOOST_CHECK_EQUAL(actionProbs[1].action, 1);
    BOOST_CHECK_EQUAL(actionProbs[2].action, 0);

    // deviation expressed in %
    BOOST_CHECK_CLOSE(actionProbs[0].probability, 0.665, 0.5);
    BOOST_CHECK_CLOSE(actionProbs[1].probability, 0.244, 0.5);
    BOOST_CHECK_CLOSE(actionProbs[2].probability, 0.090, 0.5);
}

BOOST_AUTO_TEST_CASE( SoftmaxMinEpsilon )
{
    SoftmaxExplorer explorer(1, 0.45);

    vector<float> scores = { 1, 2, 3 };
    DecisionServicePredictorsSimple predictor(scores);
    PredictorContainer container(&predictor);
    ActionProbabilities actionProbs = explorer.explore(container);

    BOOST_CHECK_EQUAL(actionProbs.size(), 3 );

    // sorted by score
    BOOST_CHECK_EQUAL(actionProbs[0].action, 2);
    BOOST_CHECK_EQUAL(actionProbs[1].action, 1);
    BOOST_CHECK_EQUAL(actionProbs[2].action, 0);

    // deviation expressed in %
    BOOST_CHECK_CLOSE(actionProbs[0].probability, 0.621, 0.5);
    BOOST_CHECK_CLOSE(actionProbs[1].probability, 0.228, 0.5);
    BOOST_CHECK_CLOSE(actionProbs[2].probability, 0.45/3, 0.5);
}

BOOST_AUTO_TEST_SUITE_END()