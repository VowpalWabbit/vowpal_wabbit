using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using VW;

namespace cs_vw
{
    public class Program
    {
        enum FileMode
        {
            JsonArray,

            JsonNewLine
        }

        private static FileMode DetectedFileMode(string file)
        {
            // auto-detect if it's line separated or array based
            using (var reader = new StreamReader(file))
            {
                var buffer = new char[32];
                int readChars;

                while ((readChars = reader.Read(buffer, 0, buffer.Length)) > 0)
                {
                    for (int i = 0; i < readChars; i++)
                    {
                        var c = buffer[i];
                        switch (c)
                        {
                            case ' ':
                            case '\t':
                            case '\n':
                            case '\r':
                                continue;
                            case '[':
                                return FileMode.JsonArray;
                            case '{':
                                return FileMode.JsonNewLine;
                            default:
                                throw new ArgumentException("Invalid character: " + c);
                        }
                    }
                }
            }

            throw new ArgumentException("Empty file");
        }

        public static void Main(string[] args)
        {
            // first argument needs to end with .json
            if (args.Length == 0)
            {
                Console.Error.WriteLine(
                    "Usage: {0} <input.json> <vw arg1> <vw arg2> ...",
                    Path.GetFileName(Assembly.GetExecutingAssembly().Location));
                return;
            }

            try
            {
                var json = args[0];
                var vwArguments = string.Join(" ", args.Skip(1));

                var fileMode = DetectedFileMode(json);

                using (var vw = new VowpalWabbitJson(vwArguments))
                {
                    switch (fileMode)
                    {
                        case FileMode.JsonArray:
                            using (var reader = new JsonTextReader(new StreamReader(json)))
                            {
                                if (!reader.Read())
                                    return;

                                if (reader.TokenType != JsonToken.StartArray)
                                    return;

                                while (reader.Read())
                                {
                                    switch (reader.TokenType)
                                    {
                                        case JsonToken.StartObject:
                                            vw.Learn(reader);
                                            break;
                                        case JsonToken.EndObject:
                                            // skip
                                            break;
                                        case JsonToken.EndArray:
                                            // end reading
                                            return;
                                    }
                                }
                            }
                            break;
                        case FileMode.JsonNewLine:
                            using (var reader = new StreamReader(json))
                            {
                                string line;

                                while ((line = reader.ReadLine()) != null)
                                {
                                    if (string.IsNullOrWhiteSpace(line))
                                        continue;

                                    vw.Learn(line);
                                }
                            }
                            break;
                    }
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Exception: {0}.\n{1}", e.Message, e.StackTrace);
            }
        }
    }
}
