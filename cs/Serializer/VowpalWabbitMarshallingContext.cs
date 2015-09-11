using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW.Serializer
{
    public class VowpalWabbitMarshallingContext : IDisposable
    {
        public VowpalWabbitMarshallingContext(VowpalWabbit vw)
        {
            this.VW = vw;
            this.StringExample = new StringBuilder();

            this.ExampleBuilder = new VowpalWabbitExampleBuilder(vw)
        }

        public VowpalWabbit VW { get; private set; }

        /// <summary>
        /// See https://github.com/JohnLangford/vowpal_wabbit/wiki/Input-format for reference
        /// </summary>
        public StringBuilder StringExample { get; private set; }

        /// <summary>
        /// Used to build examples. Builder is allocated deallocated in Visit.
        /// </summary>
        public VowpalWabbitExampleBuilder ExampleBuilder { get; private set; }

        public VowpalWabbitNamespaceBuilder NamespaceBuilder { get; set; }

        public void Dispose()
        {
            // TODO: dispose context
            // TOOD: dispose examplebuilder
        }
    }
}
