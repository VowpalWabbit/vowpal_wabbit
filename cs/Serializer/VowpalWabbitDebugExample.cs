using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Serializer.Visitors;

namespace VW.Serializer
{
    [DebuggerDisplay("CmdLine: {VowpalWabbitString}")]
    public class VowpalWabbitDebugExample : VowpalWabbitExample
    {
        internal VowpalWabbitDebugExample(VowpalWabbitExample example, string vwString) :
            base(example.Owner, example)
        {
            this.VowpalWabbitString = vwString;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public string VowpalWabbitString 
        {
            get;
            private set;
        }
    }
}
