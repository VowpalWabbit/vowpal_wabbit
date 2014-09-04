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

		interaction->Serialize(serialized_Stream);
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
		std::string content = serialized_Stream.str();
		
		this->Clear_Data();

		return content;
	}

private:

	void Clear_Data()
	{
		serialized_Stream.clear();
	}

private:
	std::string appId;

	std::stringstream serialized_Stream;
};