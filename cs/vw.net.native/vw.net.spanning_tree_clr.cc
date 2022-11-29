#include "vw.net.spanning_tree_clr.h"

API VW::SpanningTree* CreateSpanningTree() { return new VW::SpanningTree; }

API void DeleteSpanningTree(VW::SpanningTree* tree) { delete tree; }

API short unsigned int GetSpanningTreeBoundPort(VW::SpanningTree* tree) { return tree->BoundPort(); }

API void StartSpanningTree(VW::SpanningTree* tree) { tree->Start(); }

API void RunSpanningTree(VW::SpanningTree* tree) { tree->Run(); }

API void StopSpanningTree(VW::SpanningTree* tree) { tree->Stop(); }
