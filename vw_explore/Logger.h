//
// Logging module of the MWT service
//

// Currently based on assumption that each app stores separate files
class Logger
{
public:

	Logger(std::string appId) : appId(appId)
	{
		memInteractions.clear();
	}

	void Store(Interaction* interaction)
	{
		if (interaction == nullptr)
		{
			throw std::invalid_argument("Interaction to store is NULL");
		}
		memInteractions.push_back(interaction);

		this->AutoFlush();
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
			
			memInteractions.push_back(interactions[i]);
			
			this->AutoFlush();
		}
	}

	Interaction* Load(u64 interactionID)
	{
		Interaction* retInter = nullptr;

		// First check if it's already in memory
		// TODO: maybe index the in-mem vector by ID for faster lookups 
		for (size_t i = 0; i < memInteractions.size(); i++)
		{
			if (memInteractions[i]->GetId() == interactionID)
			{
				retInter = memInteractions[i];
				break;
			}
		}
		if (retInter == nullptr)
		{
			// TODO: load from disk if needed or contact the service
		}
		return retInter;
	}

	void Join(u64 interactionID, Reward* reward)
	{
		if (reward == nullptr)
		{
			throw new std::invalid_argument("Reward must be specified.");
		}
		Interaction* interaction = this->Load(interactionID);
		if (interaction == nullptr)
		{
			throw new std::exception("Interaction should have been stored but is not found.");
		}
		interaction->UpdateReward(reward);

		// TODO: store the updated interaction back either to memory or to file
	}

private:
	// Check and flush data to disk if necessary
	void AutoFlush()
	{
		bool shouldFlush = false;
		// TODO: check for flush criteria

		for (size_t i = 0; i < memInteractions.size(); i++)
		{
			u8* bytes = nullptr;
			int byteLength = 0;
			memInteractions[i]->Serialize(bytes, byteLength);

			// TODO: write to disk
		}
		memInteractions.clear();
	}

private:
	std::string appId;

	std::vector<Interaction*> memInteractions;
};