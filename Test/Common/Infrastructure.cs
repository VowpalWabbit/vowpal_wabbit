using MultiWorldTesting;

namespace TestCommon
{
    public class TestPolicy<TContext> : IPolicy<TContext>
    {
        public TestPolicy() : this(-1) { }

        public TestPolicy(int index)
        {
            this.index = index;
            this.ActionToChoose = uint.MaxValue;
        }

        public uint ChooseAction(TContext context)
        {
            return (this.ActionToChoose != -1) ? this.ActionToChoose : 5;
        }

        public uint ActionToChoose { get; set; }
        private int index;
    }

    public class TestSimplePolicy : IPolicy<SimpleContext>
    {
        public uint ChooseAction(SimpleContext context)
        {
            return 1;
        }
    }

    public class StringPolicy : IPolicy<SimpleContext>
    {
        public uint ChooseAction(SimpleContext context)
        {
            return 1;
        }
    }

    public class RegularTestContext : IStringContext
    {
        private int id;

        public int Id
        {
            get { return id; }
            set { id = value; }
        }

        public override string ToString()
        {
            return id.ToString();
        }
    }

    public class VariableActionTestContext : RegularTestContext, IVariableActionContext
    {
        public VariableActionTestContext(uint numberOfActions)
        {
            NumberOfActions = numberOfActions;
        }

        public uint GetNumberOfActions()
        {
            return NumberOfActions;
        }

        public uint NumberOfActions { get; set; }
    }
}
