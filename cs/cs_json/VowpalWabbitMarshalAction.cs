using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    internal interface IVowpalWabbitMarshalAction
    {
        void Marshal(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName);
    }

    internal static class VowpalWabbitMarshalActions
    {
        internal static IVowpalWabbitMarshalAction Create(double data)
        {
            return new VowpalWabbitMarshalActionImpl<double>(Marshal, data);
        }

        internal static IVowpalWabbitMarshalAction Create(string data)
        {
            return new VowpalWabbitMarshalActionImpl<string>(Marshal, data);
        }

        internal static IVowpalWabbitMarshalAction Create(long data)
        {
            return new VowpalWabbitMarshalActionImpl<long>(Marshal, data);
        }

        internal static IVowpalWabbitMarshalAction Create(bool data)
        {
            return new VowpalWabbitMarshalActionImpl<bool>(Marshal, data);
        }

        internal static IVowpalWabbitMarshalAction Create(float[] data, int length)
        {
            return new VowpalWabbitMarshalActionArrayImpl<float>(Marshal, data, length);
        }

        internal static void Marshal(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName, double val)
        {
            var feature = new PreHashedFeature(context.VW, ns, featureName);
            defaultMarshaller.MarshalFeature(context, ns, feature, val);
        }

        internal static void Marshal(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName, string val)
        {
            var feature = new Feature(featureName);
            defaultMarshaller.MarshalFeatureStringEscapeAndIncludeName(context, ns, feature, val);
        }

        internal static void Marshal(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName, bool val)
        {
            var feature = new PreHashedFeature(context.VW, ns, featureName);
            defaultMarshaller.MarshalFeature(context, ns, feature, val);
        }

        internal static void Marshal(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName, long val)
        {
            var feature = new PreHashedFeature(context.VW, ns, featureName);
            defaultMarshaller.MarshalFeature(context, ns, feature, val);
        }

        internal static void Marshal(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName, float[] values, int length)
        {
            var feature = new Feature(featureName);
            defaultMarshaller.MarshalFeature(context, ns, feature, values, 0, length);
        }

        /// <summary>
        /// Explicit closure to enable debug view
        /// </summary>
        private sealed class VowpalWabbitMarshalActionImpl<T> : IVowpalWabbitMarshalAction
        {
            internal delegate void MarshalAction(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName, T data);

            private readonly T data;

            private readonly MarshalAction marshal;

            internal VowpalWabbitMarshalActionImpl(MarshalAction marshal, T data)
            {
                this.data = data;
                this.marshal = marshal;
            }

            public void Marshal(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName)
            {
                this.marshal(defaultMarshaller, context, ns, featureName, this.data);
            }
        }

        /// <summary>
        /// Explicit closure to enable debug view
        /// </summary>
        private sealed class VowpalWabbitMarshalActionArrayImpl<T> : IVowpalWabbitMarshalAction
        {
            internal delegate void MarshalAction(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName, T[] data, int length);

            private readonly T[] data;

            private readonly int length;

            private readonly MarshalAction marshal;

            internal VowpalWabbitMarshalActionArrayImpl(
                MarshalAction marshal,
                T[] data,
                int length)
            {
                this.data = data;
                this.length = length;
                this.marshal = marshal;
            }

            public void Marshal(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName)
            {
                this.marshal(defaultMarshaller, context, ns, featureName, this.data, this.length);
            }
        }
    }

    internal delegate void VowpalWabbitMarshalAction(VowpalWabbitDefaultMarshaller defaultMarshaller, VowpalWabbitMarshalContext context, Namespace ns, string featureName);
}
