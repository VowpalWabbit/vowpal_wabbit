using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
using VW.Serializer;

namespace VW
{
    public class VowpalWabbitDynamic : IDisposable
    {
        private Dictionary<Type, VowpalWabbitSerializer<object>> serializers;

        private VowpalWabbit vw;

        public VowpalWabbitDynamic(string arguments) : this(new VowpalWabbitSettings(arguments))
        {
        }

        public VowpalWabbitDynamic(VowpalWabbitSettings settings)
        {
            this.vw = new VowpalWabbit(settings);
        }

        private VowpalWabbitSerializer<object> GetOrCreateSerializer(Type type)
        {
            VowpalWabbitSerializer<object> serializer;
            if (!this.serializers.TryGetValue(type, out serializer))
            {
                var allFeatures = AnnotationInspector.ExtractFeatures(type, (_,__) => true);
                foreach (var feature in allFeatures)
                {
                    // inject type cast to the actual type (always works)
                    // needed since the serializer is generated for "type", not for "object"
                    feature.ValueExpressionFactory = expr => feature.ValueExpressionFactory(Expression.Convert(expr, type));
                }

                serializer = VowpalWabbitSerializerFactory
                    .CreateSerializer<object>(this.vw.Settings.ShallowCopy(allFeatures: allFeatures))
                    .Create(this.vw);

                this.serializers.Add(type, serializer);
            }

            return serializer;
        }

        public void Learn(object example, ILabel label)
        {
            using (var ex = GetOrCreateSerializer(example.GetType()).Serialize(example, label))
            {
                this.vw.Learn(ex);
            }
        }

        /// <summary>
        /// The wrapped VW instance.
        /// </summary>
        public VowpalWabbit Native { get { return this.vw; } }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.serializers != null)
                {
                    foreach (var serializer in this.serializers)
                    {
                        serializer.Value.Dispose();
                    }
                    this.serializers = null;
                }

                if (this.vw != null)
                {
                    this.vw.Dispose();
                    this.vw = null;
                }
            }
        }

    }
}
