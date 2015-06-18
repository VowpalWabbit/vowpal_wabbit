
namespace MultiWorldTesting
{
    /// <summary>
	/// A sample context class that stores a vector of Features.
	/// </summary>
	public class SimpleContext : IStringContext
	{
        public SimpleContext(Feature[] features)
		{
            this.Features = features;
		}

        public override string ToString()
		{
            return null;
		}

        public Feature[] GetFeatures() { return Features; }

		internal Feature[] Features;
	};
}
