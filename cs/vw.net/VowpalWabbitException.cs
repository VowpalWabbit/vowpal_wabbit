using System;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Security.Permissions;

namespace VW
{
  [Serializable]
  public class VowpalWabbitException : Exception
  {
    private const string LineNumberSerializationKey = nameof(VowpalWabbitException.LineNumber);
    private const string FilenameSerializationKey = nameof(VowpalWabbitException.Filename);

    public VowpalWabbitException(string message) : base(message)
    {
    }

    public VowpalWabbitException(string message, Exception innerException) : base(message, innerException)
    {
    }

    protected VowpalWabbitException(SerializationInfo info, StreamingContext context) : base(info, context)
    {
      this.LineNumber = info.GetInt32(LineNumberSerializationKey);
      this.Filename = info.GetString(FilenameSerializationKey);
    }

    [SecurityPermission(SecurityAction.Demand, SerializationFormatter = true)]
    public override void GetObjectData(SerializationInfo info, StreamingContext context)
    {
      if (info == null)
      {
        throw new ArgumentNullException(nameof(info));
      }

      info.AddValue(LineNumberSerializationKey, this.LineNumber);
      info.AddValue(FilenameSerializationKey, this.Filename);
      base.GetObjectData(info, context);
    }

    public string Filename { get; protected set; } = null;
    public int LineNumber { get; protected set; } = -1;
  }
}