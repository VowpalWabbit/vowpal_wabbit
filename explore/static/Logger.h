//
// Logging module of the MWT service
//

#pragma once
#include <sstream>

// Currently based on assumption that each app stores separate files
class Logger
{
public:

	Logger(std::string app_id);
	~Logger();

	void Store(Interaction* interaction);
	void Store(std::vector<Interaction*> interactions);

	std::string Get_All_Interactions();
	void Get_All_Interactions(size_t& num_interactions, Interaction**& interactions);

private:

	void Clear_Data();

private:
	std::string m_app_id;

	std::vector<Interaction*> m_interactions;
};