#include "vw.net.spanning_tree_clr.h"

API VW::spanning_tree* CreateSpanningTree() { return new VW::spanning_tree; }

API void DeleteSpanningTree(VW::spanning_tree* tree) { delete tree; }

API short unsigned int GetSpanningTreeBoundPort(VW::spanning_tree* tree) { return tree->bound_port(); }

API void StartSpanningTree(VW::spanning_tree* tree) { tree->start(); }

API void RunSpanningTree(VW::spanning_tree* tree) { tree->run(); }

API void StopSpanningTree(VW::spanning_tree* tree) { tree->stop(); }
