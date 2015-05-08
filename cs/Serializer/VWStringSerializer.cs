using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Security;
using System.Security.Permissions;
using System.Text;
using System.Threading.Tasks;
using VowpalWabbit.Serializer.Visitor;

namespace VowpalWabbit.Serializer
{
    public static class VWStringSerializer
    {
        public static string Serialize<T>(T value)
        {
            var serializer = VWSerializer.CreateSerializer<T, VowpalWabbitStringVisitor>();

            var stringVisitor = new VowpalWabbitStringVisitor();
            serializer(value, stringVisitor);

            return stringVisitor.Example;
            //return VWSerializer.ExtractExample(value).ToString();
        }
    }
}
