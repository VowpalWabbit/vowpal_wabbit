//
// Logging module of the MWT service
//

// Currently based on assumption that each app stores separate files
class Logger
{
public:

	Logger(std::string appId) : appId(appId)
	{
	}

	void Store(Interaction* interaction)
	{

	}

	void Store(std::vector<Interaction*> interactions)
	{
		if (interactions.size() == 0)
		{
			return;
		}
		for (int i = 0; i < interactions.size(); i++)
		{
			u8* bytes = nullptr;
			int byteLength = 0;
			if (interactions[i] != nullptr)
			{
				interactions[i]->Serialize(bytes, byteLength);
				// TODO: write bytes to file
			}
		}
	}

	Interaction* Load(int appId, u64 interactionID)
	{

	}

	void Join(u64 interactionID, Reward* reward)
	{

	}

private:
	std::string appId;

};