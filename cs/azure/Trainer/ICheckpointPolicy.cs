namespace VowpalWabbit.Azure.Trainer
{
    public interface ICheckpointPolicy
    {
        bool ShouldCheckpointAfterExample(int examples);

        void Reset();
    }
}
