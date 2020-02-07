// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "spanning_tree_clr.h"
#include "vw_clr.h"

using namespace std;

namespace VW
{
SpanningTreeClr::SpanningTreeClr()
{ m_spanningTree = new SpanningTree;
}

SpanningTreeClr::~SpanningTreeClr()
{ try
  { delete m_spanningTree;
  }
  CATCHRETHROW
}

void SpanningTreeClr::Start()
{ try
  { m_spanningTree->Start();
  }
  CATCHRETHROW
}

void SpanningTreeClr::Stop()
{ try
  { m_spanningTree->Stop();
  }
  CATCHRETHROW
}

void SpanningTreeClr::Run()
{ try
  { m_spanningTree->Run();
  }
  CATCHRETHROW
}
}
