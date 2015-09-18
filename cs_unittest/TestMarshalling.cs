using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class TestMarshalling
    {
        [TestMethod]
        public void TestEnumerize()
        {
            using(var vw = new VowpalWabbitExampleValidator<ExampleEnum>(string.Empty))
            {
                vw.Validate("|Age25", new ExampleEnum() { Age = 25 });
                vw.Validate("|Age0 Age2:25", new ExampleEnum() { Age2 = 25 });
                vw.Validate("|Age0 Age2:25 AgeEnumChild", new ExampleEnum() { Age2 = 23, AgeEnum = Age.Child });
            }
        }

        [TestMethod]
        public void TestString()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString>(string.Empty))
            {
                vw.Validate("|abc London", new ExampleString() { Location = "London" });
                vw.Validate("", new ExampleString() { });
            }
        }

        [TestMethod]
        [ExpectedException(typeof(AssertFailedException))]
        public void TestStringInvalid()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString>(string.Empty))
            {
                // this is an example of incompatibility between C# and VowpalWabbit string format due to missing escape syntax
                vw.Validate("|abc New York", new ExampleString() { Location = "New York" });
            }
        }
    }

    public class ExampleEnum
    {
        //[Feature(Enumerize = true)]
        public int Age { get; set; }

        //[Feature]
        public int? Age2 { get; set; }

        [Feature]
        public Age? AgeEnum { get; set; }
    }


    public class ExampleString
    {
        [Feature(FeatureGroup = 'a', Namespace = "bc")]
        public string Location { get; set; }
    }

    public enum Age
    {
        Child,
        Adult
    }
}
