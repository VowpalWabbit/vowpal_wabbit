/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include <boost/test/unit_test.hpp>
#include <vector>

#include "ds_explore.h"

using namespace std;
#include "utility.h"

using namespace boost::unit_test;
using namespace Microsoft::DecisionService;
using namespace MultiWorldTesting;

BOOST_AUTO_TEST_SUITE( DecisionServiceActionProbabilities )

BOOST_AUTO_TEST_CASE( ActionProbabilities_init )
{
    ActionProbabilities actionProbs(3);

    BOOST_CHECK_EQUAL(actionProbs[0].action, 0);
    BOOST_CHECK_EQUAL(actionProbs[1].action, 1);
    BOOST_CHECK_EQUAL(actionProbs[2].action, 2);
}

BOOST_AUTO_TEST_CASE( ActionProbabilities_swap_first )
{
    ActionProbabilities actionProbs(3);
    actionProbs[0].probability = 0.8;
    actionProbs[1].probability = 0.1;
    actionProbs[2].probability = 0.1;

    actionProbs.swap_first(2);

    BOOST_CHECK_EQUAL(actionProbs[0].action, 2);
    BOOST_CHECK_EQUAL(actionProbs[1].action, 1);
    BOOST_CHECK_EQUAL(actionProbs[2].action, 0);

    BOOST_CHECK_CLOSE(actionProbs[0].probability, 0.1, 0.00001);
    BOOST_CHECK_CLOSE(actionProbs[1].probability, 0.1, 0.00001);
    BOOST_CHECK_CLOSE(actionProbs[2].probability, 0.8, 0.00001);
}

BOOST_AUTO_TEST_CASE( ActionProbabilities_sort_by_probabilities_desc )
{
    ActionProbabilities actionProbs(3);
    actionProbs[0].probability = 0.1;
    actionProbs[1].probability = 0.1;
    actionProbs[2].probability = 0.8;

    actionProbs.sort_by_probabilities_desc();

    BOOST_CHECK_EQUAL(actionProbs[0].action, 2);
    BOOST_CHECK_EQUAL(actionProbs[1].action, 0);
    BOOST_CHECK_EQUAL(actionProbs[2].action, 1);

    BOOST_CHECK_CLOSE(actionProbs[0].probability, 0.8, 0.00001);
    BOOST_CHECK_CLOSE(actionProbs[1].probability, 0.1, 0.00001);
    BOOST_CHECK_CLOSE(actionProbs[2].probability, 0.1, 0.00001);
}

BOOST_AUTO_TEST_CASE( ActionProbabilities_sample )
{
    ActionProbabilities actionProbs(3);
    actionProbs[0].probability = 0.7;
    actionProbs[1].probability = 0.2;
    actionProbs[2].probability = 0.1;

    PRG::prg random_generator(123);
    double counts[3] = { 0, 0, 0};

    int n = 10000;
    for(int i=0;i<10000;i++)
    {
        float draw = random_generator.Uniform_Unit_Interval();

        counts[actionProbs.sample(draw)]++;
    }

    for (int i=0;i<3;i++)
        counts[i] /= n;

    BOOST_CHECK_CLOSE(counts[0], 0.7, 2.f);
    BOOST_CHECK_CLOSE(counts[1], 0.2, 2.f);
    BOOST_CHECK_CLOSE(counts[2], 0.1, 2.f);
}

// TODO: probabilities, actions, saftey

BOOST_AUTO_TEST_SUITE_END()