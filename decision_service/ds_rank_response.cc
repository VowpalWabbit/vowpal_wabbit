/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "ds_api.h"
#include <sstream>

namespace Microsoft {
  namespace DecisionService {

    using namespace std;

    RankResponse::RankResponse(std::vector<int>&& ranking, const char* pevent_id, const char* pmodel_version, std::vector<float>&& pprobabilities, const char* pfeatures)
      : _ranking(ranking), event_id(pevent_id), model_version(pmodel_version), _probabilities(pprobabilities), features(pfeatures)
    {
      // check that ranking has more than 0 elements
    }

    int RankResponse::length()
    {
      return _ranking.size();
    }

    int RankResponse::top_action()
    {
      // we always have at least 1 element
      return _ranking[0];
    }

    int RankResponse::action(int index)
    {
      if (index < 0 || index >= _ranking.size())
        return -1; // TODO: throw argument exception

      return _ranking[index];
    }

    float RankResponse::probability(int index)
    {
      if (index < 0 || index >= _probabilities.size())
        return -1; // TODO: throw argument exception

      return _probabilities[index];
    }

    const std::vector<float>& RankResponse::probabilities() { return _probabilities; }

    const std::vector<int>& RankResponse::ranking() { return _ranking; }

    std::ostream& operator<<(std::ostream& ostr, const RankResponse& rankResponse)
    {
      // JSON format
      // {"Version":"1","EventId":"b94a280e32024acb9a4fa12b058157d3","a":[13,11,7,2,1,5,9,3,10,6,8,14,4,12]
      // ,"c":{"Geo":{"country":"United States","_countrycf":"8",...},
      // "p":[0.814285755,0.0142857144,0.0142857144,0.0142857144,0.0142857144,0.0142857144,0.0142857144,0.0142857144,0.0142857144,0.0142857144,0.0142857144,0.0142857144,0.0142857144,0.0142857144],
      // "VWState":{"m":"680ec362b798463eaf64489efaa0d7b1/d13d8ad87e6f4af3b090b0f9cb9faaac"}}

      // TODO: locales?
      ostr << "{\"Version\":\"1\",\"EventId\":\"" << rankResponse.event_id << "\","
        "\"a\":[";

      // insert ranking
      for (auto& r : rankResponse._ranking)
        ostr << r << ",";
      // remove last ,
      ostr.seekp(-1, ostr.cur);

      ostr << "],\"c\":{" << rankResponse.features << "},"
        "\"p\":[";

      // insert ranking
      for (auto& r : rankResponse._probabilities)
        ostr << r << ",";
      // remove last ,
      ostr.seekp(-1, ostr.cur);

      // include model version
      ostr << "],\"VWState\":{\"m\":\"" << rankResponse.model_version << "\"}}";
      return ostr;
    }
  }
}
