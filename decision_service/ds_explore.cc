#include "ds_explore.h"

#include <algorithm>

namespace Microsoft {
  namespace DecisionService {
      using namespace std;

    // IExplorer::IExplorer()
    // { }

    IExplorer::~IExplorer()
    { }

    ActionProbabilities IExplorer::explore(PredictorContainer& container)
    {
      return ActionProbabilities(0);
    }
  }
}