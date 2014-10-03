#include "stdafx.h"

Logger::Logger(std::string app_id) : m_app_id(app_id)
{
	this->Clear_Data();
}

Logger::~Logger()
{
	// If the logger is deleted while still having in-mem data then try flushing it
	if (m_interactions.size() > 0)
	{
		throw std::exception("Logger still has data during destruction");
	}
}

void Logger::Store(Interaction* interaction)
{
	if (interaction == nullptr)
	{
		throw std::invalid_argument("Interaction to store is NULL");
	}

	m_interactions.push_back(interaction->Copy());
}

void Logger::Store(std::vector<Interaction*> interactions)
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

std::string Logger::Get_All_Interactions()
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

void Logger::Get_All_Interactions(size_t& num_interactions, Interaction**& interactions)
{
	num_interactions = m_interactions.size();
	interactions = new Interaction*[num_interactions];
	for (size_t i = 0; i < m_interactions.size(); i++)
	{
		 interactions[i] = m_interactions[i];
	}
	m_interactions.clear();
}

void Logger::Clear_Data()
{
	for (size_t i = 0; i < m_interactions.size(); i++)
	{
		 delete m_interactions[i];
	}
	m_interactions.clear();
}