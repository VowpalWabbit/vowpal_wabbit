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
                vw.Validate("| AgeEnumerize25", new ExampleEnum() { AgeEnumerize = 25 });
                vw.Validate("| AgeEnumerize0 AgeNumeric:25", new ExampleEnum() { AgeNumeric = 25 });
                vw.Validate("| AgeEnumerize0 AgeNumeric:23 AgeEnumChild", new ExampleEnum() { AgeNumeric = 23, AgeEnum = Age.Child });
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
        public void TestStringFeatureGroup()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString2>(string.Empty))
            {
                vw.Validate("|a London", new ExampleString2() { Location = "London" });
            }
        }

        [TestMethod]
        [ExpectedException(typeof(AssertFailedException))]
        public void TestStringNamespace()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString3>(string.Empty))
            {
                // this is an example of incompatibility between C# and VowpalWabbit string format due to missing escape syntax
                vw.Validate("| London", new ExampleString3() { Location = "London" });
            }
        }

        [TestMethod]
        [ExpectedException(typeof(AssertFailedException))]
        public void TestStringInvalid()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString>(string.Empty))
            {
                // this is an example of incompatibility between C# and VowpalWabbit string format due to missing escape syntax
                vw.Validate("| abc New York", new ExampleString() { Location = "New York" });
            }
        }
    }

    public class ExampleEnum
    {
        [Feature(Enumerize = true)]
        public int AgeEnumerize { get; set; }

        [Feature]
        public int? AgeNumeric { get; set; }

        [Feature]
        public Age? AgeEnum { get; set; }
    }


    public class ExampleString
    {
        [Feature(FeatureGroup = 'a', Namespace = "bc")]
        public string Location { get; set; }
    }

    public class ExampleString2
    {
        [Feature(FeatureGroup = 'a')]
        public string Location { get; set; }
    }

    public class ExampleString3
    {
        [Feature(Namespace = "bc")]
        public string Location { get; set; }
    }

    public enum Age
    {
        Child,
        Adult
    }
}
