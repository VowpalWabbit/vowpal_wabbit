#pragma once

#include "vw.net.native.h"
#include "spanning_tree.h"

extern "C" {
  API VW::SpanningTree* CreateSpanningTree();
  API void DeleteSpanningTree(VW::SpanningTree* tree);

  API short unsigned int GetSpanningTreeBoundPort(VW::SpanningTree* tree);
  API void StartSpanningTree(VW::SpanningTree* tree);
  API void RunSpanningTree(VW::SpanningTree* tree);
  API void StopSpanningTree(VW::SpanningTree* tree);
}
