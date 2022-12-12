using System;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Security.Permissions;
using System.Text;
using System.Text.RegularExpressions;
using Vw.Net.Native;

namespace Vw.Net
{
  [Serializable]
  public class VWException : VW.VowpalWabbitException
  {
    private static readonly Regex KnownFieldRegex = new Regex(@"^\(ERR:10000\) vw_exception:  (?<message>[^(]*)\(\w+(?<file_name>[^:]*[^:\w])\w+:\w+(?<line_number>\d*)\w+\)$", RegexOptions.Compiled);
    private const string ErrorCodeSerializationKey = nameof(VWException.ErrorCode);

    public int ErrorCode
    {
      get;
      set;
    } = -1;

    private static string ExtractVwExceptionFields(ApiStatus status, out string fileName, out int lineNumber)
    {
      if (status.ErrorCode == 10000)
      {
        // The message has the following format:
        // "(ERR:10000), vw_exception: , can't open: models_out/0001.model.writing, errno = No such file or directory, (, io_adapter.cc, :, 303, )"
        // "(ERR: " <ErrorCode> ")," (ErrorName) ": ," <ActualMessage> ", (" <FileName> ", : ," <LineNumber> ", )" 
        Regex MessageRegex = new Regex(@"^\(ERR:(?<error_code>\d+)\), (?<error_name>[^:]+): , (?<actual_message>.+?(?=, \()), \(, (?<file_name>.+?(?=, :,)), :, (?<line_number>[^,]+), \)$");
        // the format of message is (ex.what()) "(" (file_name) ":" (line_number) ")"
        // extract out the raw message, file name and line number
        var match = MessageRegex.Match(status.ErrorMessage);
        if (match.Success)
        {
          fileName = match.Groups["file_name"].Value;
          lineNumber = int.Parse(match.Groups["line_number"].Value);
          return match.Groups["actual_message"].Value;
        }
      }

      fileName = null;
      lineNumber = -1;
      return status.ErrorMessage;
    }

    public VWException(ApiStatus status) : base(ExtractVwExceptionFields(status, out string fileName, out int lineNumber))
    {
      this.ErrorCode = status.ErrorCode;

      this.Filename = fileName;
      this.LineNumber = lineNumber;
    }

    // public VWException(string message) : base(NativeMethods.OpaqueBindingErrorMessage + message)
    // {
    //   this.ErrorCode = NativeMethods.OpaqueBindingError;
    // }

    public VWException(ApiStatus status, Exception inner) : base(status.ErrorMessage, inner)
    {
      this.ErrorCode = status.ErrorCode;
    }

    protected VWException(SerializationInfo info, StreamingContext context) : base(info, context)
    {
      this.ErrorCode = info.GetInt32(ErrorCodeSerializationKey);
    }

    [SecurityPermission(SecurityAction.Demand, SerializationFormatter = true)]
    public override void GetObjectData(SerializationInfo info, StreamingContext context)
    {
      if (info == null)
      {
        throw new ArgumentNullException(nameof(info));
      }

      info.AddValue(ErrorCodeSerializationKey, this.ErrorCode);
      base.GetObjectData(info, context);
    }
  }
}