using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW.Serializer
{
    public enum StringProcessing
    {
        /// <summary>
        /// Spaces are replaced with underscores.
        /// </summary>
        Escape,

        /// <summary>
        /// Strings are split on space, producing individual features.
        /// </summary>
        Split,

        /// <summary>
        /// Spaces are replaced with underscores and the property name is used as a prefix.
        /// </summary>
        EscapeAndIncludeName
    }
}
