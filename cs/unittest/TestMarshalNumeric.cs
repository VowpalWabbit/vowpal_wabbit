
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer;
using VW.Serializer.Attributes;

namespace cs_unittest
{
	[TestClass]
    public class TestMarshalNumeric
    {
	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleByte>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleByte() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericByteArray()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleByteArray>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleByteArray() { Value = new System.Byte[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleByteArray());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericByteArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleByteArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleByteArrayAnchor() { Value = new System.Byte[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleByteArrayAnchor());
            }
		}

	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericSByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleSByte>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleSByte() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericSByteArray()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleSByteArray>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleSByteArray() { Value = new System.SByte[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleSByteArray());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericSByteArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleSByteArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleSByteArrayAnchor() { Value = new System.SByte[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleSByteArrayAnchor());
            }
		}

	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleInt16>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleInt16() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericInt16Array()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleInt16Array>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleInt16Array() { Value = new System.Int16[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleInt16Array());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericInt16ArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleInt16ArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleInt16ArrayAnchor() { Value = new System.Int16[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleInt16ArrayAnchor());
            }
		}

	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleInt32>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleInt32() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericInt32Array()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleInt32Array>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleInt32Array() { Value = new System.Int32[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleInt32Array());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericInt32ArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleInt32ArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleInt32ArrayAnchor() { Value = new System.Int32[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleInt32ArrayAnchor());
            }
		}

	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericUInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleUInt16>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleUInt16() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericUInt16Array()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleUInt16Array>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleUInt16Array() { Value = new System.UInt16[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleUInt16Array());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericUInt16ArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleUInt16ArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleUInt16ArrayAnchor() { Value = new System.UInt16[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleUInt16ArrayAnchor());
            }
		}

	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericUInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleUInt32>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleUInt32() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericUInt32Array()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleUInt32Array>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleUInt32Array() { Value = new System.UInt32[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleUInt32Array());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericUInt32ArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleUInt32ArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleUInt32ArrayAnchor() { Value = new System.UInt32[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleUInt32ArrayAnchor());
            }
		}

	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericSingle()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleSingle>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleSingle() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericSingleArray()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleSingleArray>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleSingleArray() { Value = new System.Single[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleSingleArray());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericSingleArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleSingleArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleSingleArrayAnchor() { Value = new System.Single[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleSingleArrayAnchor());
            }
		}

	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericInt64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleInt64>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleInt64() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericInt64Array()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleInt64Array>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleInt64Array() { Value = new System.Int64[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleInt64Array());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericInt64ArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleInt64ArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleInt64ArrayAnchor() { Value = new System.Int64[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleInt64ArrayAnchor());
            }
		}

	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericUInt64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleUInt64>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleUInt64() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericUInt64Array()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleUInt64Array>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleUInt64Array() { Value = new System.UInt64[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleUInt64Array());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericUInt64ArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleUInt64ArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleUInt64ArrayAnchor() { Value = new System.UInt64[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleUInt64ArrayAnchor());
            }
		}

	
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericDouble()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleDouble>(string.Empty))
            {
                vw.Validate("| Value:25", new NumericExampleDouble() { Value = 25 });
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericDoubleArray()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleDoubleArray>(string.Empty))
            {
                vw.Validate("| :4 :2 :3", new NumericExampleDoubleArray() { Value = new System.Double[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleDoubleArray());
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestNumericDoubleArrayAnchor()
		{
		    using(var vw = new VowpalWabbitExampleValidator<NumericExampleDoubleArrayAnchor>(string.Empty))
            {
                vw.Validate("| :1 :4 :2 :3", new NumericExampleDoubleArrayAnchor() { Value = new System.Double[] { 4, 2, 3 } });
                vw.Validate("", new NumericExampleDoubleArrayAnchor());
            }
		}

	
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByte>(string.Empty))
            {
				var example = new DictionaryExampleByte() { Dict = new Dictionary<System.Byte, float>() };

				example.Dict.Add(15, .5f);
				example.Dict.Add(5, .3f);
				example.Dict.Add(20, 123.2f);

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteString()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteString>(string.Empty))
            {
				var example = new DictionaryExampleByteString() { Dict = new Dictionary<System.Byte, string>() };

				example.Dict.Add(15, "0.5");
				example.Dict.Add(5, "0.3");
				example.Dict.Add(20, "123.2");

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteByte>(string.Empty))
            {
				var example = new DictionaryExampleByteByte() { Dict = new Dictionary<System.Byte, Byte>() };

				example.Dict.Add(15, (Byte)3);
				example.Dict.Add(5, (Byte)4);
				example.Dict.Add(20, (Byte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteSByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteSByte>(string.Empty))
            {
				var example = new DictionaryExampleByteSByte() { Dict = new Dictionary<System.Byte, SByte>() };

				example.Dict.Add(15, (SByte)3);
				example.Dict.Add(5, (SByte)4);
				example.Dict.Add(20, (SByte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteInt16>(string.Empty))
            {
				var example = new DictionaryExampleByteInt16() { Dict = new Dictionary<System.Byte, Int16>() };

				example.Dict.Add(15, (Int16)3);
				example.Dict.Add(5, (Int16)4);
				example.Dict.Add(20, (Int16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteInt32>(string.Empty))
            {
				var example = new DictionaryExampleByteInt32() { Dict = new Dictionary<System.Byte, Int32>() };

				example.Dict.Add(15, (Int32)3);
				example.Dict.Add(5, (Int32)4);
				example.Dict.Add(20, (Int32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteUInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteUInt16>(string.Empty))
            {
				var example = new DictionaryExampleByteUInt16() { Dict = new Dictionary<System.Byte, UInt16>() };

				example.Dict.Add(15, (UInt16)3);
				example.Dict.Add(5, (UInt16)4);
				example.Dict.Add(20, (UInt16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteUInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteUInt32>(string.Empty))
            {
				var example = new DictionaryExampleByteUInt32() { Dict = new Dictionary<System.Byte, UInt32>() };

				example.Dict.Add(15, (UInt32)3);
				example.Dict.Add(5, (UInt32)4);
				example.Dict.Add(20, (UInt32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteSingle()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteSingle>(string.Empty))
            {
				var example = new DictionaryExampleByteSingle() { Dict = new Dictionary<System.Byte, Single>() };

				example.Dict.Add(15, (Single)3);
				example.Dict.Add(5, (Single)4);
				example.Dict.Add(20, (Single)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteInt64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteInt64>(string.Empty))
            {
				var example = new DictionaryExampleByteInt64() { Dict = new Dictionary<System.Byte, Int64>() };

				example.Dict.Add(15, (Int64)3);
				example.Dict.Add(5, (Int64)4);
				example.Dict.Add(20, (Int64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteUInt64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteUInt64>(string.Empty))
            {
				var example = new DictionaryExampleByteUInt64() { Dict = new Dictionary<System.Byte, UInt64>() };

				example.Dict.Add(15, (UInt64)3);
				example.Dict.Add(5, (UInt64)4);
				example.Dict.Add(20, (UInt64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryByteDouble()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleByteDouble>(string.Empty))
            {
				var example = new DictionaryExampleByteDouble() { Dict = new Dictionary<System.Byte, Double>() };

				example.Dict.Add(15, (Double)3);
				example.Dict.Add(5, (Double)4);
				example.Dict.Add(20, (Double)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByte>(string.Empty))
            {
				var example = new DictionaryExampleSByte() { Dict = new Dictionary<System.SByte, float>() };

				example.Dict.Add(15, .5f);
				example.Dict.Add(5, .3f);
				example.Dict.Add(20, 123.2f);

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteString()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteString>(string.Empty))
            {
				var example = new DictionaryExampleSByteString() { Dict = new Dictionary<System.SByte, string>() };

				example.Dict.Add(15, "0.5");
				example.Dict.Add(5, "0.3");
				example.Dict.Add(20, "123.2");

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteByte>(string.Empty))
            {
				var example = new DictionaryExampleSByteByte() { Dict = new Dictionary<System.SByte, Byte>() };

				example.Dict.Add(15, (Byte)3);
				example.Dict.Add(5, (Byte)4);
				example.Dict.Add(20, (Byte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteSByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteSByte>(string.Empty))
            {
				var example = new DictionaryExampleSByteSByte() { Dict = new Dictionary<System.SByte, SByte>() };

				example.Dict.Add(15, (SByte)3);
				example.Dict.Add(5, (SByte)4);
				example.Dict.Add(20, (SByte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteInt16>(string.Empty))
            {
				var example = new DictionaryExampleSByteInt16() { Dict = new Dictionary<System.SByte, Int16>() };

				example.Dict.Add(15, (Int16)3);
				example.Dict.Add(5, (Int16)4);
				example.Dict.Add(20, (Int16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteInt32>(string.Empty))
            {
				var example = new DictionaryExampleSByteInt32() { Dict = new Dictionary<System.SByte, Int32>() };

				example.Dict.Add(15, (Int32)3);
				example.Dict.Add(5, (Int32)4);
				example.Dict.Add(20, (Int32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteUInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteUInt16>(string.Empty))
            {
				var example = new DictionaryExampleSByteUInt16() { Dict = new Dictionary<System.SByte, UInt16>() };

				example.Dict.Add(15, (UInt16)3);
				example.Dict.Add(5, (UInt16)4);
				example.Dict.Add(20, (UInt16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteUInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteUInt32>(string.Empty))
            {
				var example = new DictionaryExampleSByteUInt32() { Dict = new Dictionary<System.SByte, UInt32>() };

				example.Dict.Add(15, (UInt32)3);
				example.Dict.Add(5, (UInt32)4);
				example.Dict.Add(20, (UInt32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteSingle()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteSingle>(string.Empty))
            {
				var example = new DictionaryExampleSByteSingle() { Dict = new Dictionary<System.SByte, Single>() };

				example.Dict.Add(15, (Single)3);
				example.Dict.Add(5, (Single)4);
				example.Dict.Add(20, (Single)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteInt64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteInt64>(string.Empty))
            {
				var example = new DictionaryExampleSByteInt64() { Dict = new Dictionary<System.SByte, Int64>() };

				example.Dict.Add(15, (Int64)3);
				example.Dict.Add(5, (Int64)4);
				example.Dict.Add(20, (Int64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteUInt64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteUInt64>(string.Empty))
            {
				var example = new DictionaryExampleSByteUInt64() { Dict = new Dictionary<System.SByte, UInt64>() };

				example.Dict.Add(15, (UInt64)3);
				example.Dict.Add(5, (UInt64)4);
				example.Dict.Add(20, (UInt64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionarySByteDouble()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleSByteDouble>(string.Empty))
            {
				var example = new DictionaryExampleSByteDouble() { Dict = new Dictionary<System.SByte, Double>() };

				example.Dict.Add(15, (Double)3);
				example.Dict.Add(5, (Double)4);
				example.Dict.Add(20, (Double)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16>(string.Empty))
            {
				var example = new DictionaryExampleInt16() { Dict = new Dictionary<System.Int16, float>() };

				example.Dict.Add(15, .5f);
				example.Dict.Add(5, .3f);
				example.Dict.Add(20, 123.2f);

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16String()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16String>(string.Empty))
            {
				var example = new DictionaryExampleInt16String() { Dict = new Dictionary<System.Int16, string>() };

				example.Dict.Add(15, "0.5");
				example.Dict.Add(5, "0.3");
				example.Dict.Add(20, "123.2");

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16Byte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16Byte>(string.Empty))
            {
				var example = new DictionaryExampleInt16Byte() { Dict = new Dictionary<System.Int16, Byte>() };

				example.Dict.Add(15, (Byte)3);
				example.Dict.Add(5, (Byte)4);
				example.Dict.Add(20, (Byte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16SByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16SByte>(string.Empty))
            {
				var example = new DictionaryExampleInt16SByte() { Dict = new Dictionary<System.Int16, SByte>() };

				example.Dict.Add(15, (SByte)3);
				example.Dict.Add(5, (SByte)4);
				example.Dict.Add(20, (SByte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16Int16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16Int16>(string.Empty))
            {
				var example = new DictionaryExampleInt16Int16() { Dict = new Dictionary<System.Int16, Int16>() };

				example.Dict.Add(15, (Int16)3);
				example.Dict.Add(5, (Int16)4);
				example.Dict.Add(20, (Int16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16Int32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16Int32>(string.Empty))
            {
				var example = new DictionaryExampleInt16Int32() { Dict = new Dictionary<System.Int16, Int32>() };

				example.Dict.Add(15, (Int32)3);
				example.Dict.Add(5, (Int32)4);
				example.Dict.Add(20, (Int32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16UInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16UInt16>(string.Empty))
            {
				var example = new DictionaryExampleInt16UInt16() { Dict = new Dictionary<System.Int16, UInt16>() };

				example.Dict.Add(15, (UInt16)3);
				example.Dict.Add(5, (UInt16)4);
				example.Dict.Add(20, (UInt16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16UInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16UInt32>(string.Empty))
            {
				var example = new DictionaryExampleInt16UInt32() { Dict = new Dictionary<System.Int16, UInt32>() };

				example.Dict.Add(15, (UInt32)3);
				example.Dict.Add(5, (UInt32)4);
				example.Dict.Add(20, (UInt32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16Single()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16Single>(string.Empty))
            {
				var example = new DictionaryExampleInt16Single() { Dict = new Dictionary<System.Int16, Single>() };

				example.Dict.Add(15, (Single)3);
				example.Dict.Add(5, (Single)4);
				example.Dict.Add(20, (Single)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16Int64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16Int64>(string.Empty))
            {
				var example = new DictionaryExampleInt16Int64() { Dict = new Dictionary<System.Int16, Int64>() };

				example.Dict.Add(15, (Int64)3);
				example.Dict.Add(5, (Int64)4);
				example.Dict.Add(20, (Int64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16UInt64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16UInt64>(string.Empty))
            {
				var example = new DictionaryExampleInt16UInt64() { Dict = new Dictionary<System.Int16, UInt64>() };

				example.Dict.Add(15, (UInt64)3);
				example.Dict.Add(5, (UInt64)4);
				example.Dict.Add(20, (UInt64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt16Double()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt16Double>(string.Empty))
            {
				var example = new DictionaryExampleInt16Double() { Dict = new Dictionary<System.Int16, Double>() };

				example.Dict.Add(15, (Double)3);
				example.Dict.Add(5, (Double)4);
				example.Dict.Add(20, (Double)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32>(string.Empty))
            {
				var example = new DictionaryExampleInt32() { Dict = new Dictionary<System.Int32, float>() };

				example.Dict.Add(15, .5f);
				example.Dict.Add(5, .3f);
				example.Dict.Add(20, 123.2f);

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32String()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32String>(string.Empty))
            {
				var example = new DictionaryExampleInt32String() { Dict = new Dictionary<System.Int32, string>() };

				example.Dict.Add(15, "0.5");
				example.Dict.Add(5, "0.3");
				example.Dict.Add(20, "123.2");

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32Byte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32Byte>(string.Empty))
            {
				var example = new DictionaryExampleInt32Byte() { Dict = new Dictionary<System.Int32, Byte>() };

				example.Dict.Add(15, (Byte)3);
				example.Dict.Add(5, (Byte)4);
				example.Dict.Add(20, (Byte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32SByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32SByte>(string.Empty))
            {
				var example = new DictionaryExampleInt32SByte() { Dict = new Dictionary<System.Int32, SByte>() };

				example.Dict.Add(15, (SByte)3);
				example.Dict.Add(5, (SByte)4);
				example.Dict.Add(20, (SByte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32Int16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32Int16>(string.Empty))
            {
				var example = new DictionaryExampleInt32Int16() { Dict = new Dictionary<System.Int32, Int16>() };

				example.Dict.Add(15, (Int16)3);
				example.Dict.Add(5, (Int16)4);
				example.Dict.Add(20, (Int16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32Int32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32Int32>(string.Empty))
            {
				var example = new DictionaryExampleInt32Int32() { Dict = new Dictionary<System.Int32, Int32>() };

				example.Dict.Add(15, (Int32)3);
				example.Dict.Add(5, (Int32)4);
				example.Dict.Add(20, (Int32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32UInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32UInt16>(string.Empty))
            {
				var example = new DictionaryExampleInt32UInt16() { Dict = new Dictionary<System.Int32, UInt16>() };

				example.Dict.Add(15, (UInt16)3);
				example.Dict.Add(5, (UInt16)4);
				example.Dict.Add(20, (UInt16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32UInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32UInt32>(string.Empty))
            {
				var example = new DictionaryExampleInt32UInt32() { Dict = new Dictionary<System.Int32, UInt32>() };

				example.Dict.Add(15, (UInt32)3);
				example.Dict.Add(5, (UInt32)4);
				example.Dict.Add(20, (UInt32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32Single()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32Single>(string.Empty))
            {
				var example = new DictionaryExampleInt32Single() { Dict = new Dictionary<System.Int32, Single>() };

				example.Dict.Add(15, (Single)3);
				example.Dict.Add(5, (Single)4);
				example.Dict.Add(20, (Single)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32Int64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32Int64>(string.Empty))
            {
				var example = new DictionaryExampleInt32Int64() { Dict = new Dictionary<System.Int32, Int64>() };

				example.Dict.Add(15, (Int64)3);
				example.Dict.Add(5, (Int64)4);
				example.Dict.Add(20, (Int64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32UInt64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32UInt64>(string.Empty))
            {
				var example = new DictionaryExampleInt32UInt64() { Dict = new Dictionary<System.Int32, UInt64>() };

				example.Dict.Add(15, (UInt64)3);
				example.Dict.Add(5, (UInt64)4);
				example.Dict.Add(20, (UInt64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryInt32Double()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleInt32Double>(string.Empty))
            {
				var example = new DictionaryExampleInt32Double() { Dict = new Dictionary<System.Int32, Double>() };

				example.Dict.Add(15, (Double)3);
				example.Dict.Add(5, (Double)4);
				example.Dict.Add(20, (Double)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16>(string.Empty))
            {
				var example = new DictionaryExampleUInt16() { Dict = new Dictionary<System.UInt16, float>() };

				example.Dict.Add(15, .5f);
				example.Dict.Add(5, .3f);
				example.Dict.Add(20, 123.2f);

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16String()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16String>(string.Empty))
            {
				var example = new DictionaryExampleUInt16String() { Dict = new Dictionary<System.UInt16, string>() };

				example.Dict.Add(15, "0.5");
				example.Dict.Add(5, "0.3");
				example.Dict.Add(20, "123.2");

                vw.Validate("| 15:0.5 5:0.3 20:123.2", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}

	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16Byte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16Byte>(string.Empty))
            {
				var example = new DictionaryExampleUInt16Byte() { Dict = new Dictionary<System.UInt16, Byte>() };

				example.Dict.Add(15, (Byte)3);
				example.Dict.Add(5, (Byte)4);
				example.Dict.Add(20, (Byte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16SByte()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16SByte>(string.Empty))
            {
				var example = new DictionaryExampleUInt16SByte() { Dict = new Dictionary<System.UInt16, SByte>() };

				example.Dict.Add(15, (SByte)3);
				example.Dict.Add(5, (SByte)4);
				example.Dict.Add(20, (SByte)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16Int16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16Int16>(string.Empty))
            {
				var example = new DictionaryExampleUInt16Int16() { Dict = new Dictionary<System.UInt16, Int16>() };

				example.Dict.Add(15, (Int16)3);
				example.Dict.Add(5, (Int16)4);
				example.Dict.Add(20, (Int16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16Int32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16Int32>(string.Empty))
            {
				var example = new DictionaryExampleUInt16Int32() { Dict = new Dictionary<System.UInt16, Int32>() };

				example.Dict.Add(15, (Int32)3);
				example.Dict.Add(5, (Int32)4);
				example.Dict.Add(20, (Int32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16UInt16()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16UInt16>(string.Empty))
            {
				var example = new DictionaryExampleUInt16UInt16() { Dict = new Dictionary<System.UInt16, UInt16>() };

				example.Dict.Add(15, (UInt16)3);
				example.Dict.Add(5, (UInt16)4);
				example.Dict.Add(20, (UInt16)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16UInt32()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16UInt32>(string.Empty))
            {
				var example = new DictionaryExampleUInt16UInt32() { Dict = new Dictionary<System.UInt16, UInt32>() };

				example.Dict.Add(15, (UInt32)3);
				example.Dict.Add(5, (UInt32)4);
				example.Dict.Add(20, (UInt32)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16Single()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16Single>(string.Empty))
            {
				var example = new DictionaryExampleUInt16Single() { Dict = new Dictionary<System.UInt16, Single>() };

				example.Dict.Add(15, (Single)3);
				example.Dict.Add(5, (Single)4);
				example.Dict.Add(20, (Single)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16Int64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16Int64>(string.Empty))
            {
				var example = new DictionaryExampleUInt16Int64() { Dict = new Dictionary<System.UInt16, Int64>() };

				example.Dict.Add(15, (Int64)3);
				example.Dict.Add(5, (Int64)4);
				example.Dict.Add(20, (Int64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16UInt64()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16UInt64>(string.Empty))
            {
				var example = new DictionaryExampleUInt16UInt64() { Dict = new Dictionary<System.UInt16, UInt64>() };

				example.Dict.Add(15, (UInt64)3);
				example.Dict.Add(5, (UInt64)4);
				example.Dict.Add(20, (UInt64)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
		public void TestDictionaryUInt16Double()
		{
		    using(var vw = new VowpalWabbitExampleValidator<DictionaryExampleUInt16Double>(string.Empty))
            {
				var example = new DictionaryExampleUInt16Double() { Dict = new Dictionary<System.UInt16, Double>() };

				example.Dict.Add(15, (Double)3);
				example.Dict.Add(5, (Double)4);
				example.Dict.Add(20, (Double)5);

                vw.Validate("| 15:3 5:4 20:5", example);

				example.Dict = null;
                vw.Validate("", example);
            }
		}
	
	
    }

		public class NumericExampleByte
	{
		[Feature]
		public System.Byte Value { get; set; }
	}

	public class NumericExampleByteArray
	{
		[Feature]
		public System.Byte[] Value { get; set; }
	}

	public class NumericExampleByteArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.Byte[] Value { get; set; }
	}

		public class NumericExampleSByte
	{
		[Feature]
		public System.SByte Value { get; set; }
	}

	public class NumericExampleSByteArray
	{
		[Feature]
		public System.SByte[] Value { get; set; }
	}

	public class NumericExampleSByteArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.SByte[] Value { get; set; }
	}

		public class NumericExampleInt16
	{
		[Feature]
		public System.Int16 Value { get; set; }
	}

	public class NumericExampleInt16Array
	{
		[Feature]
		public System.Int16[] Value { get; set; }
	}

	public class NumericExampleInt16ArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.Int16[] Value { get; set; }
	}

		public class NumericExampleInt32
	{
		[Feature]
		public System.Int32 Value { get; set; }
	}

	public class NumericExampleInt32Array
	{
		[Feature]
		public System.Int32[] Value { get; set; }
	}

	public class NumericExampleInt32ArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.Int32[] Value { get; set; }
	}

		public class NumericExampleUInt16
	{
		[Feature]
		public System.UInt16 Value { get; set; }
	}

	public class NumericExampleUInt16Array
	{
		[Feature]
		public System.UInt16[] Value { get; set; }
	}

	public class NumericExampleUInt16ArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.UInt16[] Value { get; set; }
	}

		public class NumericExampleUInt32
	{
		[Feature]
		public System.UInt32 Value { get; set; }
	}

	public class NumericExampleUInt32Array
	{
		[Feature]
		public System.UInt32[] Value { get; set; }
	}

	public class NumericExampleUInt32ArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.UInt32[] Value { get; set; }
	}

		public class NumericExampleSingle
	{
		[Feature]
		public System.Single Value { get; set; }
	}

	public class NumericExampleSingleArray
	{
		[Feature]
		public System.Single[] Value { get; set; }
	}

	public class NumericExampleSingleArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.Single[] Value { get; set; }
	}

		public class NumericExampleInt64
	{
		[Feature]
		public System.Int64 Value { get; set; }
	}

	public class NumericExampleInt64Array
	{
		[Feature]
		public System.Int64[] Value { get; set; }
	}

	public class NumericExampleInt64ArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.Int64[] Value { get; set; }
	}

		public class NumericExampleUInt64
	{
		[Feature]
		public System.UInt64 Value { get; set; }
	}

	public class NumericExampleUInt64Array
	{
		[Feature]
		public System.UInt64[] Value { get; set; }
	}

	public class NumericExampleUInt64ArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.UInt64[] Value { get; set; }
	}

		public class NumericExampleDouble
	{
		[Feature]
		public System.Double Value { get; set; }
	}

	public class NumericExampleDoubleArray
	{
		[Feature]
		public System.Double[] Value { get; set; }
	}

	public class NumericExampleDoubleArrayAnchor
	{
		[Feature(AddAnchor = true)]
		public System.Double[] Value { get; set; }
	}

	
		public class DictionaryExampleByte
	{
		[Feature]
		public Dictionary<System.Byte, float> Dict { get; set; }
	}

	public class DictionaryExampleByteString
	{
		[Feature]
		public Dictionary<System.Byte, String> Dict { get; set; }
	}

		public class DictionaryExampleByteByte
	{
		[Feature]
		public Dictionary<System.Byte, System.Byte> Dict { get; set; }
	}
		public class DictionaryExampleByteSByte
	{
		[Feature]
		public Dictionary<System.Byte, System.SByte> Dict { get; set; }
	}
		public class DictionaryExampleByteInt16
	{
		[Feature]
		public Dictionary<System.Byte, System.Int16> Dict { get; set; }
	}
		public class DictionaryExampleByteInt32
	{
		[Feature]
		public Dictionary<System.Byte, System.Int32> Dict { get; set; }
	}
		public class DictionaryExampleByteUInt16
	{
		[Feature]
		public Dictionary<System.Byte, System.UInt16> Dict { get; set; }
	}
		public class DictionaryExampleByteUInt32
	{
		[Feature]
		public Dictionary<System.Byte, System.UInt32> Dict { get; set; }
	}
		public class DictionaryExampleByteSingle
	{
		[Feature]
		public Dictionary<System.Byte, System.Single> Dict { get; set; }
	}
		public class DictionaryExampleByteInt64
	{
		[Feature]
		public Dictionary<System.Byte, System.Int64> Dict { get; set; }
	}
		public class DictionaryExampleByteUInt64
	{
		[Feature]
		public Dictionary<System.Byte, System.UInt64> Dict { get; set; }
	}
		public class DictionaryExampleByteDouble
	{
		[Feature]
		public Dictionary<System.Byte, System.Double> Dict { get; set; }
	}
			public class DictionaryExampleSByte
	{
		[Feature]
		public Dictionary<System.SByte, float> Dict { get; set; }
	}

	public class DictionaryExampleSByteString
	{
		[Feature]
		public Dictionary<System.SByte, String> Dict { get; set; }
	}

		public class DictionaryExampleSByteByte
	{
		[Feature]
		public Dictionary<System.SByte, System.Byte> Dict { get; set; }
	}
		public class DictionaryExampleSByteSByte
	{
		[Feature]
		public Dictionary<System.SByte, System.SByte> Dict { get; set; }
	}
		public class DictionaryExampleSByteInt16
	{
		[Feature]
		public Dictionary<System.SByte, System.Int16> Dict { get; set; }
	}
		public class DictionaryExampleSByteInt32
	{
		[Feature]
		public Dictionary<System.SByte, System.Int32> Dict { get; set; }
	}
		public class DictionaryExampleSByteUInt16
	{
		[Feature]
		public Dictionary<System.SByte, System.UInt16> Dict { get; set; }
	}
		public class DictionaryExampleSByteUInt32
	{
		[Feature]
		public Dictionary<System.SByte, System.UInt32> Dict { get; set; }
	}
		public class DictionaryExampleSByteSingle
	{
		[Feature]
		public Dictionary<System.SByte, System.Single> Dict { get; set; }
	}
		public class DictionaryExampleSByteInt64
	{
		[Feature]
		public Dictionary<System.SByte, System.Int64> Dict { get; set; }
	}
		public class DictionaryExampleSByteUInt64
	{
		[Feature]
		public Dictionary<System.SByte, System.UInt64> Dict { get; set; }
	}
		public class DictionaryExampleSByteDouble
	{
		[Feature]
		public Dictionary<System.SByte, System.Double> Dict { get; set; }
	}
			public class DictionaryExampleInt16
	{
		[Feature]
		public Dictionary<System.Int16, float> Dict { get; set; }
	}

	public class DictionaryExampleInt16String
	{
		[Feature]
		public Dictionary<System.Int16, String> Dict { get; set; }
	}

		public class DictionaryExampleInt16Byte
	{
		[Feature]
		public Dictionary<System.Int16, System.Byte> Dict { get; set; }
	}
		public class DictionaryExampleInt16SByte
	{
		[Feature]
		public Dictionary<System.Int16, System.SByte> Dict { get; set; }
	}
		public class DictionaryExampleInt16Int16
	{
		[Feature]
		public Dictionary<System.Int16, System.Int16> Dict { get; set; }
	}
		public class DictionaryExampleInt16Int32
	{
		[Feature]
		public Dictionary<System.Int16, System.Int32> Dict { get; set; }
	}
		public class DictionaryExampleInt16UInt16
	{
		[Feature]
		public Dictionary<System.Int16, System.UInt16> Dict { get; set; }
	}
		public class DictionaryExampleInt16UInt32
	{
		[Feature]
		public Dictionary<System.Int16, System.UInt32> Dict { get; set; }
	}
		public class DictionaryExampleInt16Single
	{
		[Feature]
		public Dictionary<System.Int16, System.Single> Dict { get; set; }
	}
		public class DictionaryExampleInt16Int64
	{
		[Feature]
		public Dictionary<System.Int16, System.Int64> Dict { get; set; }
	}
		public class DictionaryExampleInt16UInt64
	{
		[Feature]
		public Dictionary<System.Int16, System.UInt64> Dict { get; set; }
	}
		public class DictionaryExampleInt16Double
	{
		[Feature]
		public Dictionary<System.Int16, System.Double> Dict { get; set; }
	}
			public class DictionaryExampleInt32
	{
		[Feature]
		public Dictionary<System.Int32, float> Dict { get; set; }
	}

	public class DictionaryExampleInt32String
	{
		[Feature]
		public Dictionary<System.Int32, String> Dict { get; set; }
	}

		public class DictionaryExampleInt32Byte
	{
		[Feature]
		public Dictionary<System.Int32, System.Byte> Dict { get; set; }
	}
		public class DictionaryExampleInt32SByte
	{
		[Feature]
		public Dictionary<System.Int32, System.SByte> Dict { get; set; }
	}
		public class DictionaryExampleInt32Int16
	{
		[Feature]
		public Dictionary<System.Int32, System.Int16> Dict { get; set; }
	}
		public class DictionaryExampleInt32Int32
	{
		[Feature]
		public Dictionary<System.Int32, System.Int32> Dict { get; set; }
	}
		public class DictionaryExampleInt32UInt16
	{
		[Feature]
		public Dictionary<System.Int32, System.UInt16> Dict { get; set; }
	}
		public class DictionaryExampleInt32UInt32
	{
		[Feature]
		public Dictionary<System.Int32, System.UInt32> Dict { get; set; }
	}
		public class DictionaryExampleInt32Single
	{
		[Feature]
		public Dictionary<System.Int32, System.Single> Dict { get; set; }
	}
		public class DictionaryExampleInt32Int64
	{
		[Feature]
		public Dictionary<System.Int32, System.Int64> Dict { get; set; }
	}
		public class DictionaryExampleInt32UInt64
	{
		[Feature]
		public Dictionary<System.Int32, System.UInt64> Dict { get; set; }
	}
		public class DictionaryExampleInt32Double
	{
		[Feature]
		public Dictionary<System.Int32, System.Double> Dict { get; set; }
	}
			public class DictionaryExampleUInt16
	{
		[Feature]
		public Dictionary<System.UInt16, float> Dict { get; set; }
	}

	public class DictionaryExampleUInt16String
	{
		[Feature]
		public Dictionary<System.UInt16, String> Dict { get; set; }
	}

		public class DictionaryExampleUInt16Byte
	{
		[Feature]
		public Dictionary<System.UInt16, System.Byte> Dict { get; set; }
	}
		public class DictionaryExampleUInt16SByte
	{
		[Feature]
		public Dictionary<System.UInt16, System.SByte> Dict { get; set; }
	}
		public class DictionaryExampleUInt16Int16
	{
		[Feature]
		public Dictionary<System.UInt16, System.Int16> Dict { get; set; }
	}
		public class DictionaryExampleUInt16Int32
	{
		[Feature]
		public Dictionary<System.UInt16, System.Int32> Dict { get; set; }
	}
		public class DictionaryExampleUInt16UInt16
	{
		[Feature]
		public Dictionary<System.UInt16, System.UInt16> Dict { get; set; }
	}
		public class DictionaryExampleUInt16UInt32
	{
		[Feature]
		public Dictionary<System.UInt16, System.UInt32> Dict { get; set; }
	}
		public class DictionaryExampleUInt16Single
	{
		[Feature]
		public Dictionary<System.UInt16, System.Single> Dict { get; set; }
	}
		public class DictionaryExampleUInt16Int64
	{
		[Feature]
		public Dictionary<System.UInt16, System.Int64> Dict { get; set; }
	}
		public class DictionaryExampleUInt16UInt64
	{
		[Feature]
		public Dictionary<System.UInt16, System.UInt64> Dict { get; set; }
	}
		public class DictionaryExampleUInt16Double
	{
		[Feature]
		public Dictionary<System.UInt16, System.Double> Dict { get; set; }
	}
		}
