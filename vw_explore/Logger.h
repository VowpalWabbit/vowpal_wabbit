//
// Logging module of the MWT service
//

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
			this->Auto_Flush();
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

		this->Auto_Flush();
	}

	void Store(std::vector<Interaction*> interactions)
	{
		if (interactions.size() == 0)
		{
			throw std::invalid_argument("Interaction set to store is empty");
		}
		for (size_t i = 0; i < interactions.size(); i++)
		{
			if (interactions[i] == nullptr)
			{
				throw std::invalid_argument("Interaction set to store is empty");
			}
			
			memory_Interactions.push_back(interactions[i]->Copy());
			
			this->Auto_Flush();
		}
	}

	Interaction* Load(u64 interaction_Id)
	{
		Interaction* return_Interaction = nullptr;

		// First check if it's already in memory
		// TODO: maybe index the in-mem vector by ID for faster lookups 
		for (size_t i = 0; i < memory_Interactions.size(); i++)
		{
			if (memory_Interactions[i]->Get_Id() == interaction_Id)
			{
				return_Interaction = memory_Interactions[i];
				break;
			}
		}
		if (return_Interaction == nullptr)
		{
			// TODO: load from disk if needed or contact the service
		}
		return return_Interaction;
	}

	void Join(u64 interaction_Id, Reward* reward)
	{
		if (reward == nullptr)
		{
			throw new std::invalid_argument("Reward must be specified.");
		}
		Interaction* interaction = this->Load(interaction_Id);
		if (interaction == nullptr)
		{
			throw new std::exception("Interaction should have been stored but is not found.");
		}
		interaction->Update_Reward(reward);

		// TODO: store the updated interaction back either to memory or to file
	}

private:
	// Check and flush data to disk if necessary
	void Auto_Flush()
	{
		bool shouldFlush = false;
		// TODO: check for flush criteria

		for (size_t i = 0; i < memory_Interactions.size(); i++)
		{
			u8* bytes = nullptr;
			int byteLength = 0;
			memory_Interactions[i]->Serialize(bytes, byteLength);

			// TODO: write to disk
		}
		this->Clear_Data();
	}

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