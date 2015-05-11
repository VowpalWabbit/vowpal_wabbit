using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    public interface IVowpalWabbitVisitor<TResultExample, TResultNamespace, TResultFeature>
    {
        TResultExample Visit(string comment, IVisitableNamespace<TResultNamespace>[] namespaces);

        TResultNamespace Visit<T>(INamespaceDense<T> namespaceDense);

        TResultNamespace Visit(INamespaceSparse<TResultFeature> namespaceSparse);

        TResultFeature Visit<T>(IFeature<T> feature);

        TResultFeature VisitEnumerize<T>(IFeature<T> feature);
    }
}
