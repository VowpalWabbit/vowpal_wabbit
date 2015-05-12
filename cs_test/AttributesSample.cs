using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.Research.MachineLearning.Serializer;
using Microsoft.Research.MachineLearning.Serializer.Visitor;
using Microsoft.Research.MachineLearning;

namespace cs_test
{
    public static class AttributesSample
    {
        public static void Attributes()
        {
            var d1 = new DocumentFeature
            {
                Id = "d1",
                Time = new DateTime(2015, 1, 1),
                Value = new LDAFeatureVector { Values = new[] { 1.0, 2.0, 3.0 } }
            };

            var context = new UserContext
            {
                User = new UserFeature
                {
                    Age = Age.Adult,
                    Gender = Gender.Female,
                    Location = "WA",
                    FeatureBag = new Dictionary<string, float>
                    {
                        { "Foo", 1.1f },
                        { "Bar", 2.0f }
                    }
                },
                UserLDATopicPreference = new LDAFeatureVector { Values = new[] { 0.1, 0.2, 0.3 } },
                Documents = new[]
                {
                    d1,
                    new DocumentFeature
                    {
                        Id = "d2",
                        Time = new DateTime(2015,1,1),
                        Value = new LDAFeatureVector { Values = new [] { 1.0, 2.0, 3.0 } }
                    },
                    d1
                }
            };

            var visitor = new VowpalWabbitStringVisitor();
            var lines = VowpalWabbitSerializer
                .CreateSerializer<UserContext, VowpalWabbitStringVisitor, string, string, string>()(context, visitor);

            foreach (var line in lines)
            {
                Console.WriteLine(line);
            }
            Console.ReadKey();
        }
    }

    public class UserContext : IActionDependentFeatures<DocumentFeature>
    {
        [Feature(Namespace = "otheruser", FeatureGroup = 'o')]
        public UserFeature User { get; set; }

        [Feature(Namespace = "userlda", FeatureGroup = 'u')]
        public LDAFeatureVector UserLDATopicPreference { get; set; }

        [ActionDependentFeatures]
        ICollection<T> ActionDependentFeatures { get; }
        public IEnumerable<DocumentFeature> Documents { get; set; }
    }

    [Cacheable(EqualityComparer = typeof(DocumentFeatureEqualityComparer))]
    public class DocumentFeature
    {
        [Comment]
        public string Id { get; set; }

        public DateTime Time { get; set; }

        // If we include this, it would result in mixing dense and non-dense features.
        // [Feature]
        public string ContentProvider { get; set; }

        [Feature(Namespace = "doclda", FeatureGroup = 'd')]
        public LDAFeatureVector Value { get; set; }
    }

    public class DocumentFeatureEqualityComparer : IEqualityComparer<DocumentFeature>
    {
        public bool Equals(DocumentFeature x, DocumentFeature y)
        {
            return x.Id == y.Id &&
                   x.Time == y.Time;
            // maybe compare the full vector - not so sure on this part though
            //   x.Value.Zip(y.Value, (a, b) => a == b).All(c => c);
        }

        public int GetHashCode(DocumentFeature obj)
        {
            return obj.Id.GetHashCode() + obj.Time.GetHashCode();
        }
    }

    public enum Gender
    {
        Female,
        Male,
        Unknown
    }

    public class LDAFeatureVector
    {
        public string Compressed { get; set; }

        private double[] values;

        [Feature]
        public double[] Values
        {
            get
            {
                if (this.Compressed == null)
                {
                    // e.g. call into decompression
                }
                return this.values;
            }
            set { this.values = value; }
        }
    }

    public class UserFeature
    {
        [Feature]
        public Age Age { get; set; }

        [Feature]
        public int? Income { get; set; }

        [Feature]
        public Gender Gender { get; set; }

        [Feature]
        public string Location { get; set; }

        [Feature]
        public Dictionary<string, float> FeatureBag { get; set; }

        [Feature]
        public DayOfWeek DayOfWeek { get; set; }

        /// <summary>
        /// Will generate 24 parameters
        /// </summary>
        [Feature(Enumerize = true)]
        public int HourOfDay { get; set; }
    }

    public enum Age
    {
        Child,
        Adult,
        Elderly
    }
}
