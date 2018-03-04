/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once

namespace CSOAA {
  LEARNER::base_learner* csoaa_setup(arguments& arg);

  LEARNER::base_learner* csldf_setup(arguments& arg);
  struct csoaa;
  void finish_example(vw& all, csoaa&, example& ec);

  inline example* remove_empty_last_example(multi_ex& ec_seq)
  {
    if (ec_seq.size() > 1 && example_is_newline(*ec_seq.last()))
    {
      return ec_seq.pop();
    }
    return nullptr;
  }

  class restore_last_example
  {
  public:
    restore_last_example(multi_ex& coll, example* ex) :_coll(coll), _ex(ex) {};
    ~restore_last_example() { if (_ex != nullptr) _coll.push_back(_ex); };
  private:
    multi_ex& _coll;
    example* _ex;
  };
}

namespace LabelDict
{
bool ec_is_example_header(example& ec);// example headers look like "0:-1" or just "shared"
}
