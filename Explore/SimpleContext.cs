using System.Linq;
using System.Globalization;
using System.Text;

namespace MultiWorldTesting
{
    /// <summary>
	/// A sample context class that stores a vector of Features.
	/// </summary>
	public class SimpleContext : IStringContext
	{
        public SimpleContext(Feature[] features)
		{
            this.features = features;
		}

        public override string ToString()
		{
            return string.Join(" ", this.features.Select(f => string.Format(CultureInfo.InvariantCulture, "{0}:{1}", f.Id, f.Value)));
		}

		private Feature[] features;
	};
}
