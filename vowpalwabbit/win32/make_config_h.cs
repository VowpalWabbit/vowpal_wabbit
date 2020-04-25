using System;
using System.IO;
using System.ComponentModel;
using System.Diagnostics;

public class Program
{
    private const string VersionFilePath = @"..\version.txt";

    private static Process StartGitRevParse()
    {
        Process gitProcess = new Process();
        
        try
        {
            gitProcess.StartInfo.FileName = "git";
            gitProcess.StartInfo.Arguments = "rev-parse --short HEAD";
            gitProcess.StartInfo.WorkingDirectory = Path.GetDirectoryName(VersionFilePath);
            gitProcess.StartInfo.UseShellExecute = false; // Should we use ShellExec()?
            gitProcess.StartInfo.CreateNoWindow = true;

            gitProcess.StartInfo.RedirectStandardOutput = true;

            gitProcess.Start();

        }
        catch (Win32Exception)
        {
            gitProcess = null;
        }
        
        return gitProcess;
    }

    public static void Main(string[] args)
    {
        using (Process p = StartGitRevParse())
        try
        {
            string[] lines = File.ReadAllLines("..\\version.txt");

            if(lines.Length < 1)
            {
                throw new Exception("version.txt first line should contain the version number.");
            }

            string version = lines[0].Trim();

            string gitCommit = String.Empty;
            
            if (p != null) 
            {
                gitCommit = p.StandardOutput.ReadToEnd().Trim();
            
                // Needed?
                p.WaitForExit();
            }

            string config = "#define PACKAGE_VERSION \"" + version + "\"\n"
                          + "#define COMMIT_VERSION \"" + gitCommit + "\"\n";

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
