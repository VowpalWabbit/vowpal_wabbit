//
// Logging module of the MWT service
//

#include <sstream>

// Currently based on assumption that each app stores separate files
class Logger
{
public:

	Logger(std::string appId) : appId(appId)
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
		// TODO: maybe do a deep copy here to avoid invalid mem addresses later
		if (interaction == nullptr)
		{
			throw std::invalid_argument("Interaction to store is NULL");
		}

		memory_Interactions.push_back(interaction->Copy());
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

	std::stringstream Get_All_Interactions()
	{
		std::stringstream string_data;

		for (size_t i = 0; i < memory_Interactions.size(); i++)
		{
			memory_Interactions[i]->Serialize(string_data);
		}

		return string_data;
	}

private:

	void Clear_Data()
	{
		// Delete all heap-allocated interactions
		for (size_t i = 0; i < memory_Interactions.size(); i++)
		{
			delete memory_Interactions[i];
		}

		memory_Interactions.clear();
	}

private:
	std::string appId;

	std::vector<Interaction*> memory_Interactions;
};