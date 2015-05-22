using Microsoft.Research.MachineLearning.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning
{
    public abstract class SharedExample : IExample
    {
        private static readonly SharedLabel sharedLabel = new SharedLabel();

        public ILabel Label
        {
            get { return sharedLabel; }
        }

        internal class SharedLabel : ILabel
        {
            public string ToVowpalWabbitFormat()
            {
                return "shared";
            }
        }
    }
}
