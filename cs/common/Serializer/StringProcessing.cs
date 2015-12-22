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
        Split
    }
}
