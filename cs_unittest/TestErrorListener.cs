using Antlr4.Runtime;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    class TestErrorListener : IAntlrErrorListener<IToken>
    {
        public void SyntaxError(IRecognizer recognizer, IToken offendingSymbol, int line, int charPositionInLine, string msg, RecognitionException e)
        {
            Assert.Fail("SyntaxError: {0} at line {1} character {2}: {3}",
                offendingSymbol,
                line,
                charPositionInLine,
                msg);
        }
    }
}
