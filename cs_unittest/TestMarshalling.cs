using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections;
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
        [TestCategory("Marshal")]
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
        [TestCategory("Marshal")]
        public void TestString()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString>(string.Empty))
            {
                vw.Validate("|abc London", new ExampleString() { Location = "London" });
                vw.Validate("", new ExampleString() { });
            }
        }

        [TestMethod]
        [TestCategory("Marshal")]
        public void TestStringFeatureGroup()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString2>(string.Empty))
            {
                vw.Validate("|a London", new ExampleString2() { Location = "London" });
            }
        }

        [TestMethod]
        [ExpectedException(typeof(AssertFailedException))]
        [TestCategory("Marshal")]
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
        [TestCategory("Marshal")]
        public void TestStringInvalid()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString>(string.Empty))
            {
                // this is an example of incompatibility between C# and VowpalWabbit string format due to missing escape syntax
                vw.Validate("| abc New York", new ExampleString() { Location = "New York" });
            }
        }

        [TestMethod]
        [TestCategory("Marshal")]
        public void TestDictionary()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleDictionary>(string.Empty))
            {
                var ex = new ExampleDictionary() { Dict = new Dictionary<object, object>() };
                ex.Dict.Add("Age", 25);
                ex.Dict.Add("Location", 1.2);

                vw.Validate("| Age:25 Location:1.2", ex);
            }
        }

        [TestMethod]
        [TestCategory("Marshal")]
        public void TestCustomType()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleCustomType>(string.Empty))
            {
                vw.Validate("| Custom2", new ExampleCustomType { Custom = new CustomType { value = 2 } });
            }
        }

        [TestMethod]
        [TestCategory("Marshal")]
        public void TestComplexType()
        {
            using (var vw = new VowpalWabbitExampleValidator<UserContext>(string.Empty))
            {
                vw.Validate("|ootheruser AgeAdult GenderMale PAge25 Views:4321 Boston 6:2.4", new UserContext {
                    User = new UserFeatures
                    {
                        Age = Age.Adult,
                        Gender = Gender.Male,
                        Location = "Boston",
                        PAge = 25,
                        Views = 4321,
                        Dict = new Dictionary<int, float>
                        {
                            { 6, 2.4f }
                        }
                    }
                });

                vw.Validate("|uuserlda :1 :2 :3", new UserContext
                {
                    UserLDAVector = new FeatureVector { Vectors = new[] { 2f, 3f } }
                });
            }

            using (var vw = new VowpalWabbitExampleValidator<Document>(string.Empty))
            {
                vw.Validate("| abc |ddoclda :1 :4 :5", new Document
                    {
                        Id = "abc",
                        LDAVector = new FeatureVector { Vectors = new[] { 4f, 5f } }
                    });

                vw.Validate("| abc |ddoclda :1", new Document
                {
                    Id = "abc",
                    LDAVector = new FeatureVector { Vectors = new float[0] }
                });
            }
        }
    }

    public class UserContext
    {
        [Feature(Namespace = "otheruser", FeatureGroup = 'o')]
        public UserFeatures User { get; set; }

        [Feature(Namespace = "userlda", FeatureGroup = 'u', AddAnchor = true)]
        public FeatureVector UserLDAVector { get; set; }

        public IReadOnlyList<Document> ActionDependentFeatures { get; set; }
    }

    public class Document
    {
        [Feature]
        public string Id { get; set; }

        [Feature(Namespace = "doclda", FeatureGroup = 'd', AddAnchor = true)]
        public FeatureVector LDAVector { get; set; }
    }

    public class FeatureVector
    {
        [Feature(AddAnchor = true)]
        public float[] Vectors { get; set; }
    }

    public class UserFeatures
    {
        [Feature]
        public Age? Age { get; set; }

        [Feature(Enumerize = true)]
        public int? PAge { get; set; }

        [Feature]
        public Gender? Gender { get; set; }

        [Feature]
        public string Location { get; set; }

        [Feature]
        public long Views { get; set; }

        [Feature]
        public Dictionary<int, float> Dict { get; set; }
    }

    public enum Gender
    {
        Female,
        Male
    }

    public class CustomType
    {
        public int value;

        public override string ToString()
        {
            return value.ToString();
        }
    }

    public class ExampleCustomType
    {
        [Feature]
        public CustomType Custom { get; set; }
    }

    public class ExampleDictionary
    {
        [Feature]
        public IDictionary Dict { get; set; }
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
