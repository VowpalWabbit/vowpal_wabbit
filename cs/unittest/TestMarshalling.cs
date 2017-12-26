using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Serializer;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class TestMarshalling
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
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
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestString()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString>(string.Empty))
            {
                vw.Validate("|abc London", new ExampleString() { Location = "London" });
                vw.Validate("", new ExampleString() { });
                vw.Validate("", new ExampleString() { Location = "" });
            }

            using (var vw = new VowpalWabbitExampleValidator<ExampleString4>(string.Empty))
            {
                vw.Validate("| VideoTitleRich_Homie_Quan_-_\"Blah_Blah_Blah\"___Behind_The_Scenes", new ExampleString4 { Value = "VideoTitleRich Homie Quan - \"Blah Blah Blah\" | Behind The Scenes" });
                vw.Validate("| VideoTitleIt's_Official__Your_vibrator_Can_be_Hacked", new ExampleString4 { Value = "VideoTitleIt's Official: Your vibrator Can be Hacked" });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestStringFeatureGroup()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleString2>(string.Empty))
            {
                vw.Validate("|a London", new ExampleString2() { Location = "London" });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestStringNamespace()
        {
            try
            {
                using (var vw = new VowpalWabbitExampleValidator<ExampleString3>(string.Empty))
                {
                    vw.Validate("|bc London", new ExampleString3() { Location = "London" });
                }

                Assert.Fail("Expected ArgumentException");
            }
            catch (ArgumentException)
            {
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestStringEscape()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleStringEscape>(string.Empty))
            {
                vw.Validate("| New_York_State", new ExampleStringEscape() { Value = "New York State" });
                vw.Validate("| new_York_state", new ExampleStringEscape() { Value = "new York state" });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestStringSplit()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleStringSplit>(string.Empty))
            {
                vw.Validate("| New York State", new ExampleStringSplit() { Value = "New York State" });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestStringIncludeName()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleStringInclude>(string.Empty))
            {
                vw.Validate("| AgeTeenager", new ExampleStringInclude() { Age = "Teenager" });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestDictionary()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleDictionary>(string.Empty))
            {
                var ex = new ExampleDictionary() { Dict = new Dictionary<object, object>() };
                ex.Dict.Add("Age", 25);
                ex.Dict.Add("Location", 1.2);

                vw.Validate("| Age:25 Location:1.2", ex);

                ex.Dict = null;
                vw.Validate("", ex);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestCustomType()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleCustomType>(string.Empty))
            {
                vw.Validate("| value:2", new ExampleCustomType { Custom = new CustomType { value = 2 } });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestEnumerableString()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleEnumerable>(string.Empty))
            {
                vw.Validate("| A New_York B", new ExampleEnumerable { Value = new[] { "A", "New_York", "B" } });

                vw.Validate("", new ExampleEnumerable());
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestEnumerableKV()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleEnumerableKV>(string.Empty))
            {
                vw.Validate("| A:2 B:3", new ExampleEnumerableKV
                {
                    Value = new []
                    {
                        new KeyValuePair<string, float>("A", 2),
                        new KeyValuePair<string, float>("B", 3)
                    }
                });

                vw.Validate("", new ExampleEnumerableKV());
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
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

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestEnumerizePosition()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleEnumerize>(string.Empty))
            {
                vw.Validate("| Position0", new ExampleEnumerize { Position = 0 });
                vw.Validate("| Position2", new ExampleEnumerize { Position = 2 });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestBool()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExampleBoolean>(string.Empty))
            {
                vw.Validate("| OnOff", new ExampleBoolean { OnOff = true });
                vw.Validate("| ", new ExampleBoolean { OnOff = false });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        
        public void TestFeatureDiscoveryAll()
        {
            using (var vw = new VowpalWabbitExampleValidator<POCO>(new VowpalWabbitSettings { TypeInspector = TypeInspector.All }))
            {
                vw.Validate("| Feature1 Feature2:5", new POCO { Feature1 = true, Feature2 = 5 });
            }
        }
    }

    public class POCODict
    {
        public Dictionary<string, float> Features { get; set; }
    }

    public class POCO
    {
        public bool Feature1 { get; set; }

        [Feature]
        public int Feature2 { get; set; }
    }

    public class ExampleBoolean
    {
        [Feature]
        public bool OnOff { get; set; }
    }

    public class ExampleEnumerize
    {
        [Feature(Enumerize = true)]
        public int Position { get; set; }
    }

    public class ExampleStringEscape
    {
        [Feature(StringProcessing = StringProcessing.Escape)]
        public String Value { get; set; }
    }

    public class ExampleStringInclude
    {
        [Feature(StringProcessing = StringProcessing.EscapeAndIncludeName)]
        public String Age { get; set; }
    }

    public class ExampleStringSplit
    {
        [Feature(StringProcessing = StringProcessing.Split)]
        public String Value { get; set; }
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
        [Feature]
        public int value { get; set; }
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

    public class ExampleEnumerable
    {
        [Feature]
        public IEnumerable<string> Value { get; set; }
    }

    public class ExampleEnumerableKV
    {
        [Feature]
        public IEnumerable<KeyValuePair<string, float>> Value { get; set; }
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

    public class ExampleString4
    {
        [Feature(StringProcessing = StringProcessing.Escape)]
        public string Value { get; set; }
    }

    public enum Age
    {
        Child,
        Adult
    }
}
