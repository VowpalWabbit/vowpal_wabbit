using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;

namespace VW.Labels
{
    public sealed class MulticlassLabel : ILabel
    {
        public sealed class Label : ILabel
        {
            public int Class { get; set; }

            public float? Weight { get; set; }

            public string ToVowpalWabbitFormat()
            {
                var sb = new StringBuilder();
                sb.Append(this.Class.ToString(CultureInfo.InvariantCulture));

                if (this.Weight != null)
                {
                    sb.Append(':');
                    sb.Append(this.Weight.Value.ToString(CultureInfo.InvariantCulture));
                }

                return sb.ToString();
            }
        }

        public List<Label> Classes { get; set; }

        public string ToVowpalWabbitFormat()
        {
            return string.Join(" ", this.Classes.Select(l => l.ToVowpalWabbitFormat()));
        }
    }
}
