using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    public class VowpalWabbitDefaultMarshaller
    {
        private readonly bool disableStringExampleGeneration;

        public VowpalWabbitDefaultMarshaller(bool disableStringExampleGeneration)
        {
            this.disableStringExampleGeneration = disableStringExampleGeneration;
        }

        public void MarshalFeature<T>(VowpalWabbitMarshallingContext context, Namespace ns, Feature feature, Action<T> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            // TODO: handle enum
            var featureString = feature.Name + Convert.ToString(value());
            var featureHash = context.VW.HashFeature(featureString, ns.NamespaceHash);

            context.NamespaceBuilder.AddFeature(featureHash, 1f);

            if (disableStringExampleGeneration)
            {
                return;
            }

            context.StringExample.AppendFormat(
                CultureInfo.InvariantCulture,
                " {0}",
                featureString);
        }

        public void MarshalFeature(VowpalWabbitMarshallingContext context, Namespace ns, NumericFeature feature, int value)
        {
            context.NamespaceBuilder.AddFeature(feature.FeatureHash, value);

            if (disableStringExampleGeneration)
            {
                return;
            }

            context.StringExample.AppendFormat(
                CultureInfo.InvariantCulture,
                " {0}:{1}",
                feature.Name,
                value);
        }

        public void MarshalFeature(VowpalWabbitMarshallingContext context, Namespace ns, NumericFeature feature, float[] value)
        {
            if (value == null)
            {
                return;
            }

            context.NamespaceBuilder.PreAllocate(value.Count);

            var i = 0;

            // support anchor feature
            if (feature.AddAnchor)
            {
                this.namespaceBuilder.AddFeature(ns.NamespaceHash, 1);
                i++;
            }

            foreach (var v in value)
            {
                context.NamespaceBuilder.AddFeature((uint)(ns.NamespaceHash + i), v);
                i++;
            }

            if (disableStringExampleGeneration)
            {
                return;
            }

            // support anchor feature
            i = 0;
            if (feature.AddAnchor)
            {
                context.StringExample.Append(" 0:1");
                i++;
            }

            foreach (var v in value)
            {
                context.StringExample.AppendFormat(CultureInfo.InvariantCulture, " {0}:{1}", i, v);
                i++;
            }
        }

        public void MarshalNamespace(VowpalWabbitMarshallingContext context, Namespace ns, Action featureVisits)
        {
            try
            {
                // the namespace is only added on dispose, to be able to check if at least a single feature has been added
                context.NamespaceBuilder = context.ExampleBuilder.AddNamespace(ns.FeatureGroup);

                var position = 0;
                var stringExample = context.StringExample;
                if (!disableStringExampleGeneration)
                {
                    position = stringExample.Append(ns.NamespaceString).Length;
                }

                featureVisits();

                if (!disableStringExampleGeneration)
                {
                    if (position == stringExample.Length)
                    {
                        // no features added, remove namespace
                        stringExample.Length = position - ns.NamespaceString.Length;
                    }
                }
            }
            finally
            {
                if (context.NamespaceBuilder != null)
                {
                    context.NamespaceBuilder.Dispose();
                    context.NamespaceBuilder = null;
                }
            }
        }

        public void MarshalLabel(VowpalWabbitMarshallingContext context, ILabel label)
        {
            if (label == null)
            {
                return;
            }

            context.ExampleBuilder.builder.ParseLabel(label.ToVowpalWabbitFormat());

            if (disableStringExampleGeneration)
            {
                return;
            }

            // prefix with label
            context.StringExample.Append(label.ToVowpalWabbitFormat());
        }
    }
}
