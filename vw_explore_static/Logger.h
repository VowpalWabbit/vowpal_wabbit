//
// Logging module of the MWT service
//

#pragma once
#include <sstream>

// Currently based on assumption that each app stores separate files
class Logger
{
public:

	Logger(std::string app_id) : m_app_id(app_id)
	{
		this->Clear_Data();
	}

	~Logger()
	{
		// If the logger is deleted while still having in-mem data then try flushing it
		try
		{
			this->Clear_Data();
		}
		catch (std::exception) { }
	}

	void Store(Interaction* interaction)
	{
		if (interaction == nullptr)
		{
			throw std::invalid_argument("Interaction to store is NULL");
		}

		m_interactions.push_back(interaction);
	}

	void Store(std::vector<Interaction*> interactions)
	{
		if (interactions.size() == 0)
		{
			throw std::invalid_argument("Interaction set to store is empty");
		}
		for (size_t i = 0; i < interactions.size(); i++)
		{
			this->Store(interactions[i]);
		}
	}

	std::string Get_All_Interactions()
	{
		std::ostringstream serialized_stream;

		for (size_t i = 0; i < m_interactions.size(); i++)
		{
			m_interactions[i]->Serialize(serialized_stream);
		}

		std::string content = serialized_stream.str();
		
		this->Clear_Data();

		return content;
	}

	void Get_All_Interactions(size_t& num_interactions, Interaction**& interactions)
	{
		num_interactions = m_interactions.size();
		interactions = new Interaction*[num_interactions];
		for (size_t i = 0; i < m_interactions.size(); i++)
		{
			// TODO: potential perf issue here since the Copy() dynamically creates
			// memory that are non-contiguous. This could change to malloc a chunk of
			// memory and realloc if necessary.
			 interactions[i] = m_interactions[i]->Copy();
		}
	}

private:

	void Clear_Data()
	{
		m_interactions.clear();
	}

private:
	std::string m_app_id;

	std::vector<Interaction*> m_interactions;
};