using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace VW
{
  public sealed class VowpalWabbitModel : VowpalWabbitBase
  {
    public VowpalWabbitModel(string args) : this(new VowpalWabbitSettings(args))
    {
    }

    public VowpalWabbitModel(VowpalWabbitSettings settings) : base(EnsureValidSettings(settings))
    {
    }

    // This is a bit of a weird pattern where we check and throw, but otherwise pass the object,
    // through. However, due to the base constructor being responsible for constructing the native
    // instance, we should check the values before calling into base()
    private static VowpalWabbitSettings EnsureValidSettings(VowpalWabbitSettings settings)
    {
      if (settings == null)
      {
        throw new ArgumentNullException("settings");
      }

      if (settings.Model != null)
      {
        // TODO: This is technically a "breaking change" from the previous exception thrown
        // by the C#/CLI layer, but that exception type is actually wrong. We _might_ want
        // to create a mode where we are strictly-compatible with the original, but it seems
        // like it would be a user error to rely on that other exception by type.
        throw new ArgumentException("Model must be null when creating a seed model.", "settings");
      }

      // VowpalWabbitModel and VowpalWabbit instances seeded from VowpalWabbitModel
      // need to have the same "test" setting, otherwise the stride shift is different
      // and all hell breaks loose.
      if (!TestOnlyArgumentMatcher.IsMatch(settings.Arguments))
      {
        settings.Arguments += " -t";
      }

      return settings;
    }

    private readonly static Regex TestOnlyArgumentMatcher = new Regex(@"(-t|--testonly)", RegexOptions.Compiled);
  }
}