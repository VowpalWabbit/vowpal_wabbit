using VW;

namespace cs_unittest
{
    public class TestLeakClass
    {
        public void Leak()
        {
#if DEBUG
            VowpalWabbitLeakTest.Leak();
#endif
        }

        public void NoLeak()
        {
#if DEBUG
            VowpalWabbitLeakTest.NoLeak();
#endif
        }
    }
}
