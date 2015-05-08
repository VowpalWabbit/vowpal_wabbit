using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Serializer.Reflection
{
    internal sealed class TypeMatch
    {
        internal TypeMatch(int distance)
        {
            this.Distance = distance;
            this.GenericTypes = new Dictionary<Type, Type> { };
        }

        internal TypeMatch(int distance, Type genericType, Type actualType)
            : this(distance)
        {
            this.GenericTypes = new Dictionary<Type, Type> 
                {
                    { genericType, actualType }
                };
        }

        internal TypeMatch(int distance, IEnumerable<TypeMatch> typeMatches)
            : this(distance)
        {
            this.GenericTypes = typeMatches
                .Where(tm => tm.GenericTypes != null)
                .SelectMany(tm => tm.GenericTypes)
                .ToDictionary(tm => tm.Key, tm => tm.Value);
        }

        internal int Distance { get; private set; }

        internal int InterfacesImplemented { get; set; }

        internal IDictionary<Type, Type> GenericTypes { get; private set; }
    }
}
