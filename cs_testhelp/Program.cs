using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using VW;

namespace cs_testhelp
{
    class Program
    {
        static void Main(string[] mainArgs)
        {
            var vwRoot = mainArgs[0];

            DiscoverUnitTests.Discover(vwRoot);
        }
    }
}
