using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;

namespace VW.Serializer
{
    public interface IVowpalWabbitSerializer<TExample> : IDisposable
    {
        bool EnableStringExampleGeneration { get; }

        bool CachesExamples { get; }

        string SerializeToString(TExample example, ILabel label = null, int? index = null, Dictionary<string, string> dictionary = null, Dictionary<object, string> fastDictionary = null);

        VowpalWabbitExampleCollection Serialize(TExample example, ILabel label = null, int? index = null);
    }
}
