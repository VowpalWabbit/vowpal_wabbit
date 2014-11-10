//
// Main interface for clients to the MWT service.
//

#pragma once

#include "Common.h"
#include "Explorer.h"
#include <functional>
#include <tuple>

MWT_NAMESPACE {

template <class Ctx>
class MwtExplorer
{
public:
	MwtExplorer(std::string app_id, IRecorder<Ctx>& recorder) : m_recorder(recorder)
    {
		m_app_id = HashUtils::Compute_Id_Hash(app_id);
    }
 
    template <class Exp>
    u32 Choose_Action(Exp& explorer, string unique_key, Ctx& context)
    {
		u64 seed = HashUtils::Compute_Id_Hash(unique_key);

		std::tuple<u32, float, bool> action_probability_log_tuple = explorer.Choose_Action(seed + m_app_id, context);

		u32 action = std::get<0>(action_probability_log_tuple);
		float prob = std::get<1>(action_probability_log_tuple);

		if (std::get<2>(action_probability_log_tuple))
		{
			m_recorder.Record(context, action, prob, unique_key);
		}
 
        return action;
    }
 
private:
    u64 m_app_id;
	IRecorder<Ctx>& m_recorder;
};
}
