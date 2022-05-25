using System;
using System.Runtime.InteropServices;

namespace VW
{
  public static class VowpalWabbitContextualBanditUtil
  {
    [DllImport("vw.net.native.dll", EntryPoint = "GetCbUnbiasedCost")]
    public static extern float GetUnbiasedCost(uint actionObserved, uint actionTaken, float cost, float probability);
  }
}