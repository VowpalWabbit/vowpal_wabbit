using Microsoft.Research.MachineLearning;
using Microsoft.Research.MachineLearning.Interfaces;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    [TestClass]
    public class Tests
    {
        [TestMethod]
        [DeploymentItem(@"train-sets\0002.dat", "train-sets")]
        public void Test3()
        {
            //using (var vwStr = new VowpalWabbit("-k train-sets/0002.dat -f models/0002.model --invariant -c str"))
            //using (var vw = new VowpalWabbit<Data>("-k train-sets/0002.dat -f models/0002.model --invariant"))
            //using (var fr = new StreamReader(@"train-sets\0001.dat"))
            //{

            //}
        }

        public class Data : IExample
        {
            [Feature(FeatureGroup = 'T', Name = "")]
            public string T { get; set; }

            [Feature(FeatureGroup = 'f')]
            public IEnumerable<KeyValuePair<string, float>> F { get; set; }

            public ILabel Label
            {
                get;
                set;
            }
        }
    }
}
