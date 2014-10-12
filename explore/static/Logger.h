//
// Logging module of the MWT service
//

#pragma once
#include <sstream>

// Currently based on assumption that each app stores separate files
class Logger
{
private:

  void Clear_Data()
  {
    for (size_t i = 0; i < m_interactions.size(); i++)
      delete m_interactions[i];
    m_interactions.clear();
  }

public:
  
  Logger() { this->Clear_Data(); }
  
  ~Logger()
    { // If the logger is deleted while still having in-mem data then try flushing it
      if (m_interactions.size() > 0)
	{
	  cerr << "Logger still has data during destruction" << endl;
	  throw std::exception();
	}
    }

  void Store(Interaction* interaction)
  {
    if (interaction == nullptr)
      throw std::invalid_argument("Interaction to store is NULL");
    m_interactions.push_back(interaction->Copy());
  }
    
  void Store(std::vector<Interaction*> interactions)
  {
    if (interactions.size() == 0)
      throw std::invalid_argument("Interaction set to store is empty");
    for (size_t i = 0; i < interactions.size(); i++)
      this->Store(interactions[i]);
  }
  
  std::string Get_All_Interactions()
    {
      std::ostringstream serialized_stream;
      
      for (size_t i = 0; i < m_interactions.size(); i++)
	m_interactions[i]->Serialize(serialized_stream);
      
      std::string content = serialized_stream.str();
      
      this->Clear_Data();
      
      return content;
    }

  void Get_All_Interactions(size_t& num_interactions, Interaction**& interactions)
  {
    if (m_interactions.size() > 0)
      {
	num_interactions = m_interactions.size();
	interactions = new Interaction*[num_interactions];
	for (size_t i = 0; i < m_interactions.size(); i++)
	  interactions[i] = m_interactions[i];
	m_interactions.clear();
      }
    else
      {
	num_interactions = 0;
	interactions = nullptr;
      }
  }
     
 private:
  std::vector<Interaction*> m_interactions;
};
