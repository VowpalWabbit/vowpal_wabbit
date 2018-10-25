using System;
using System.IO;

public class Program
{
    public static void Main(string[] args)
    {
        try
        {
            string[] lines = File.ReadAllLines("..\\version.txt");

            if(lines.Length < 1)
            {
                throw new Exception("version.txt first line should contain the version number.");
            }

            string version = lines[0].Trim();

             string config = "#define PACKAGE_VERSION \"" + version + "\"\n";
            if (!File.Exists("config.h") ||
                string.CompareOrdinal(File.ReadAllText("config.h"), config) != 0)
            {
                File.WriteAllText("config.h", config);
            }
        }
        catch (Exception e)
        {
            Console.Error.WriteLine(e.ToString());
            Environment.Exit(1);
        }
    }
}
