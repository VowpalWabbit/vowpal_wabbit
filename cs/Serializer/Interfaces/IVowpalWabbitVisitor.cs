using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Serializer.Interfaces
{
    public interface IVowpalWabbitVisitor
    {
        void Visit<T>(INamespaceDense<T> namespaceDense);

        void Visit(INamespaceSparse namespaceSparse, Action visitFeatures);

        void Visit<T>(IFeature<T> feature);

        void Visit(string comment, INamespace[] namespaces, Action visitNamespaces);
    }
}
