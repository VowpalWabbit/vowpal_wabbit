using cs_unittest;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace cs_testhelp
{
    internal static class DiscoverUnitTests
    {
        internal static void Discover(string vwRoot)
        {
            // find all methods that are
            // - not part of types annotated by [TestClass] (as they'll already be discovered by unit test framework)
            // - contain "test" in method name
            var methodByType =
                from type in typeof(TestRunner).Assembly.GetTypes()
                where type.GetCustomAttribute<TestClassAttribute>() != null
                from method in type.GetMethods(BindingFlags.Instance | BindingFlags.Public)
                where method.Name.ToLowerInvariant().Contains("test") &&
                    method.GetParameters().Count() == 0 &&
                    method.GetCustomAttribute<TestMethodAttribute>() != null &&
                    method.GetCustomAttribute<IgnoreAttribute>() == null
                group method by method.DeclaringType into g
                select g;

            var outputFile = vwRoot + @"\..\cs\leaktest\TestWrapped.cs";
            using (var cs = new StreamWriter(outputFile))
            {
                cs.WriteLine(@"
using Microsoft.VisualStudio.TestTools.UnitTesting;
using cs_leaktest;

namespace cs_unittest
{
    [TestClass]
    public class TestWrapped : TestWrappedBase
    {");

                foreach (var g in methodByType)
                {
                    foreach (var method in g)
                    {
                        var name = method.Name;

                        var categoryAttr = method.GetCustomAttribute<TestCategoryAttribute>();

                        if (categoryAttr != null)
                        {
                            cs.WriteLine(@"
            [TestCategory({0})]", string.Join(",",  categoryAttr.TestCategories.Select(c => string.Format("\"{0}\"", c))));
                        }

                        cs.WriteLine(@"
            [TestMethod]
            public void {1}()
            {{
                 Run(""{0}"", ""{1}"");
            }}", g.Key.FullName, name);
                    }
                }

                cs.WriteLine(@"
    }
}
");
            }
        }
    }
}
