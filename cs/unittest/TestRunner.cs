using cs_testcommon;
using System;
using System.Reflection;

namespace cs_unittest
{
    /// <summary>
    /// Helper to perform leak testing
    /// </summary>
    public class TestRunner : MarshalByRefObject, ITestRunner
    {
        public string Run(string type, string method)
        {
            var testType = Type.GetType(type);
            var testObject = Activator.CreateInstance(testType);

            try
            {
                var m = testType.GetMethod(method);

                if (m == null)
                {
                    return string.Format("TestRunner: {0}.{1} not found", type, method);
                }

                m.Invoke(testObject, null);
            }
            catch (Exception ex)
            {
                var tex = ex as TargetInvocationException;
                if (tex != null)
                {
                    ex = tex.InnerException;
                }

                return string.Format("{0}\n{1}\n#-#-#-#-#-#-#{2}", ex.GetType(), ex.Message, ex.StackTrace);
            }
            finally
            {
                var disposable = testObject as IDisposable;
                if (disposable != null)
                {
                    disposable.Dispose();
                }
            }

            return null;
        }
    }
}
