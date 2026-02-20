#pragma once

#include "vw.net.native.h"
#include "vw/spanning_tree/spanning_tree.h"

extern "C"
{
  API VW::spanning_tree* CreateSpanningTree();
  API void DeleteSpanningTree(VW::spanning_tree* tree);

  API short unsigned int GetSpanningTreeBoundPort(VW::spanning_tree* tree);
  API void StartSpanningTree(VW::spanning_tree* tree);
  API void RunSpanningTree(VW::spanning_tree* tree);
  API void StopSpanningTree(VW::spanning_tree* tree);
}
