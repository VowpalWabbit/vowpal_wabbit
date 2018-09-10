using System;
using System.Runtime.InteropServices;

namespace Rl.Net {
    public static class EntryPoints
    {
        public static void DllMain(string [] args)
        {
            //Console.Out.WriteLine("Hello World! (From .NET Standard DLL)");
            
            // This is somewhat dangerous, because we need to be super careful about matching the right
            // delete operator to the pointer type (single vs. array).
            //IntPtr configPtr = DllImport.Configuration.CreateConfig();
            using (var handle = new Configuration()) { int a = 0; a++; };
        }
    }
}