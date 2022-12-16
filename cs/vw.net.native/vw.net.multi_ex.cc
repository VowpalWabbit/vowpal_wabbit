#include "vw.net.multi_ex.h"

API VW::multi_ex* CreateMultiEx() { return new VW::multi_ex; }

API void DeleteMultiEx(VW::multi_ex* multi_ex) { delete multi_ex; }

API void MultiExAddExample(VW::multi_ex* multi_ex, VW::example* ex) { multi_ex->push_back(ex); }
