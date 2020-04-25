using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace cs_unittest
{
    [TestClass]
    public class TestMarshallingOverflow
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestNumericInt64Overflow()
        {
            using (var vw = new VowpalWabbitExampleValidator<NumericExampleInt64>(string.Empty))
            {
                vw.Validate("| Value:9.22337203685477580700E+018", new NumericExampleInt64() { Value = Int64.MaxValue });
                vw.Validate("| Value:-9.22337203685477580700E+018", new NumericExampleInt64() { Value = Int64.MinValue});
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestNumericUInt64Overflow()
        {
            using (var vw = new VowpalWabbitExampleValidator<NumericExampleUInt64>(string.Empty))
            {
                vw.Validate("| Value:1.844674E+19", new NumericExampleUInt64() { Value = UInt64.MaxValue});
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestNumericDoubleOverflow()
        {
            using (var vw = new VowpalWabbitExampleValidator<NumericExampleDouble>(string.Empty))
            {
                vw.Validate("| Value:1.79769313486231570000E+308", new NumericExampleDouble() { Value = double.MaxValue });
                vw.Validate("| Value:-1.79769313486231570000E+308", new NumericExampleDouble() { Value = double.MinValue });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestNumericInt64OverflowArray()
        {
            using (var vw = new VowpalWabbitExampleValidator<NumericExampleInt64Array>(string.Empty))
            {
                vw.Validate("| 0:9.22337203685477580700E+018", new NumericExampleInt64Array() { Value = new [] { Int64.MaxValue } });
                vw.Validate("| 0:-9.22337203685477580700E+018", new NumericExampleInt64Array() { Value = new[] { Int64.MinValue } });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestNumericUInt64OverflowArray()
        {
            using (var vw = new VowpalWabbitExampleValidator<NumericExampleUInt64Array>(string.Empty))
            {
                vw.Validate("| 0:1.844674E+19", new NumericExampleUInt64Array() { Value = new[] { UInt64.MaxValue } });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestNumericDoubleOverflowArray()
        {
            using (var vw = new VowpalWabbitExampleValidator<NumericExampleDoubleArray>(string.Empty))
            {
                vw.Validate("| 0:1.79769313486231570000E+308", new NumericExampleDoubleArray() { Value = new [] { double.MaxValue } });
                vw.Validate("| 0:-1.79769313486231570000E+308", new NumericExampleDoubleArray() { Value = new [] {  double.MinValue } });
            }
        }
    }
}
