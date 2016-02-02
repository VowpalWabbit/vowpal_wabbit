// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitHttpModule.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.ComponentModel;
using System.Web;

namespace VW
{
    /// <summary>
    /// Following blog post http://blogs.msdn.com/b/jorman/archive/2007/08/31/loading-c-assemblies-in-asp-net.aspx
    /// we need to adapt the PATH variable to be able to load zlib.dll
    /// since the c runtime of the static libraries doesn't match with C++/CLI we need to dynamically link against zlib.dll
    /// this is also related to the /DELAYLOAD:zlib.dll configuration in vw_clr: project properties -> linker -> Input
    /// </summary>
    [EditorBrowsable(EditorBrowsableState.Never)]
    public class VowpalWabbitHttpModule : IHttpModule
    {
        /// <summary>
        /// Update the PATH env variable so zlib.dll can be resolved in ASP.NET scenarios.
        /// </summary>
        public void Init(HttpApplication context)
        {
            var relSearchPath = AppDomain.CurrentDomain.RelativeSearchPath;
            var path = Environment.GetEnvironmentVariable("PATH");
            if (!path.Contains(relSearchPath))
            {
                path += ";" + relSearchPath;
                System.Environment.SetEnvironmentVariable("PATH", path, EnvironmentVariableTarget.Process);
            }
        }

        /// <summary>
        /// Cleanup PATH env variable.
        /// </summary>
        public void Dispose()
        {
            var relSearchPath = ";" + AppDomain.CurrentDomain.RelativeSearchPath;
            var path = Environment.GetEnvironmentVariable("PATH");
            var index = path.IndexOf(relSearchPath);
            if (index >= 0)
            {
                // restore path
                path = path.Substring(0, index) + path.Substring(index + relSearchPath.Length);
                System.Environment.SetEnvironmentVariable("PATH", path, EnvironmentVariableTarget.Process);
            }
        }
    }
}
