using System;
using System.IO;

public class Program
{
    public static void Main(string[] args)
    {
        try
        {
            string[] lines = File.ReadAllLines("..\\configure.ac");
            foreach (var line in lines)
            {
                if (line.Contains("AC_INIT"))
                {
                    string version = line.Split('[')[2].Split(']')[0];
                    string config = "#define PACKAGE_VERSION \"" + version + "\"\n";
                    if (!File.Exists("config.h") ||
                        string.CompareOrdinal(File.ReadAllText("config.h"), config) != 0)
                    {
                        File.WriteAllText("config.h", config);
                    }
                    return;
                }
            }
            throw new Exception("can't find AC_INIT line");
        }
        catch (Exception e)
        {
            Console.Error.WriteLine(e.ToString());
            Environment.Exit(1);
        }
    }
}

// vim:sw=4:ts=4:et:ai:cindent
