/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
class activation_function
{
public :
  virtual ~activation_function(){};
  virtual inline float fire(float x) = 0;
  virtual inline float first_derivative(float x) = 0;
  virtual inline string getName() = 0;
};

LEARNER::base_learner* nn_setup(vw& all);
