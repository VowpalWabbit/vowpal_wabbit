using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_testcommon
{
    public interface ITestRunner
    {
        /// <summary>
        /// 
        /// </summary>
        /// <param name="@class"></param>
        /// <param name="method"></param>
        /// <returns>Null if ok, otherwise the test message</returns>
        string Run(string type, string method);
    }
}
