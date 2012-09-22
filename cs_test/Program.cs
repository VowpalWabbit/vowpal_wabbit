using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Microsoft.Research.MachineLearning;

namespace cs_test
{
    class Program
    {
        static void Main(string[] args)
        {
            IntPtr vw = VowpalWabbitInterface.Initialize("--hash all -q st --noconstant -i train.w");

            IntPtr example = VowpalWabbitInterface.ReadExample(vw, "|s p^the_man w^the w^man |t p^un_homme w^un w^homme");
            float score = VowpalWabbitInterface.Learn(vw, example);
            VowpalWabbitInterface.FinishExample(vw, example);

            Console.Error.WriteLine("p2 = {0}", score);

            VowpalWabbitInterface.Finish(vw);
        }
    }
}
