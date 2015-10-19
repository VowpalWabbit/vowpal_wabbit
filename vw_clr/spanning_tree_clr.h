/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "spanning_tree.h"

namespace VW
{
  public ref class SpanningTreeClr
  {
  private:
    SpanningTree* m_spanningTree;

  public:
    SpanningTreeClr();

    ~SpanningTreeClr();

    void Start();
    void Run();
    void Stop();
  };
}
