using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Web;

namespace VW
{
    /// <summary>
    /// following blog post http://blogs.msdn.com/b/jorman/archive/2007/08/31/loading-c-assemblies-in-asp-net.aspx
    /// we need to adapt the PATH variable to be able to load zlib.dll
    /// since the c runtime of the static libraries doesn't match with C++/CLI we need to dynamically link against zlib.dll
    /// this is also related to the /DELAYLOAD:zlib.dll configuration in vw_clr: project properties -> linker -> Input
    /// </summary>
    [EditorBrowsable(EditorBrowsableState.Never)]
    public class VowpalWabbitHttpModule : IHttpModule
    {
        /// <summary>
        /// Empty.
        /// </summary>
        public void Dispose()
        {
        }

        /// <summary>
        /// Update the PATH Env variable so zlib.dll can be resolved in ASP.NET scenarios.
        /// </summary>
        public void Init(HttpApplication context)
        {
            String _path = String.Concat(System.Environment.GetEnvironmentVariable("PATH"), ";", System.AppDomain.CurrentDomain.RelativeSearchPath);
            System.Environment.SetEnvironmentVariable("PATH", _path, EnvironmentVariableTarget.Process);
        }
    }
}
