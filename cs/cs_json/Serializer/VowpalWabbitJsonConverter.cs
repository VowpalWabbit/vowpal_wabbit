using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    public class VowpalWabbitJsonSerializable : IVowpalWabbitSerializable
    {
        private readonly object value;
        private readonly JsonConverter jsonConverter;

        public VowpalWabbitJsonSerializable(object value, JsonConverter jsonConverter)
        {
            this.value = value;
            this.jsonConverter = jsonConverter;
        }

        public void Marshal(VowpalWabbitMarshalContext ctx, Namespace ns, Feature feature)
        {
            var jsonSerializer = new JsonSerializer();
            using (var jsonBuilder = new VowpalWabbitJsonBuilder(ctx.VW, VowpalWabbitDefaultMarshaller.Instance, jsonSerializer))
            {
                // serialize from object to JSON
                var sb = new StringBuilder();
                using (var writer = new JsonTextWriter(new StringWriter(sb)))
                {
                    this.jsonConverter.WriteJson(writer, this.value, jsonSerializer);
                }

                // marshal from JSON to VW
                using (var reader = new JsonTextReader(new StringReader(sb.ToString())))
                {
                    jsonBuilder.Parse(reader, ctx, new Namespace(ctx.VW, feature.Name));
                }
            }
        }
    }
}
