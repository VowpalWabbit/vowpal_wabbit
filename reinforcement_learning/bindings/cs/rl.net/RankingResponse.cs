using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;
using System.Collections;

namespace Rl.Net {
    [StructLayout(LayoutKind.Sequential)]
    public struct ActionProbability
    {
        private UIntPtr actionIndex;
        private float probability;

        public long ActionIndex => (long)this.actionIndex.ToUInt64();

        public float Probability => this.probability;
    }

    public sealed class RankingResponse: NativeObject<RankingResponse>, IEnumerable<ActionProbability>
    {
        [DllImport("rl.net.native.dll")]
        private static extern IntPtr CreateRankingResponse();

        [DllImport("rl.net.native.dll")]
        private static extern void DeleteRankingResponse(IntPtr rankingResponse);

        [DllImport("rl.net.native.dll")]
        [return: MarshalAs(NativeMethods.StringMarshalling)]
        private static extern string GetRankingEventId(IntPtr rankingResponse);
        
        [DllImport("rl.net.native.dll")]
        [return: MarshalAs(NativeMethods.StringMarshalling)]
        private static extern string GetRankingModelId(IntPtr rankingResponse);

        // TODO: CLS-compliance requires that we not publically expose unsigned types.
        // Probably not a big issue ("9e18 actions ought to be enough for anyone...")
        [DllImport("rl.net.native.dll")]
        private static extern UIntPtr GetRankingActionCount(IntPtr rankingResponse);

        [DllImport("rl.net.native.dll")]
        private static extern int GetRankingChosenAction(IntPtr rankingResponse, out UIntPtr action_id, IntPtr status);

        public RankingResponse() : base(new New<RankingResponse>(CreateRankingResponse), new Delete<RankingResponse>(DeleteRankingResponse))
        {
        }

        public string EventId => GetRankingEventId(this.NativeHandle);

        public string ModelId => GetRankingModelId(this.NativeHandle);

        public long Count
        {
            get
            {
                ulong unsignedSize = GetRankingActionCount(this.NativeHandle).ToUInt64();
                Debug.Assert(unsignedSize < Int64.MaxValue, "We do not support collections with size larger than _I64_MAX/Int64.MaxValue");
    
                return (long)unsignedSize;
            }
        }

        // TODO: Why does this method call, which seems like a "get" of a value, have an API status?
        public bool TryGetChosenAction(out long actionIndex, ApiStatus status = null)
        {
            actionIndex = -1;
            UIntPtr chosenAction;
            int result = GetRankingChosenAction(this.NativeHandle, out chosenAction, status.ToNativeHandleOrNullptr());

            if (result != NativeMethods.SuccessStatus)
            {
                return false;
            }

            actionIndex = (long)(chosenAction.ToUInt64());
            return true;
        }

        public IEnumerator<ActionProbability> GetEnumerator()
        {
            return new RankingResponseEnumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        private class RankingResponseEnumerator : NativeObject<RankingResponseEnumerator>, IEnumerator<ActionProbability>
        {
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr CreateRankingEnumeratorAdapter(IntPtr rankingResponse);

            private static New<RankingResponseEnumerator> BindConstructorArguments(RankingResponse rankingResponse)
            {
                return new New<RankingResponseEnumerator>(() => CreateRankingEnumeratorAdapter(rankingResponse.NativeHandle));
            }

            [DllImport("rl.net.native.dll")]
            private static extern void DeleteRankingEnumeratorAdapter(IntPtr rankingEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int RankingEnumeratorInit(IntPtr rankingEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int RankingEnumeratorMoveNext(IntPtr rankingEnumeratorAdapter);
        
            [DllImport("rl.net.native.dll")]
            private static extern ActionProbability GetRankingEnumeratorCurrent(IntPtr rankingEnumeratorAdapter);

            private bool initialState = true;

            public RankingResponseEnumerator(RankingResponse rankingResponse) : base(BindConstructorArguments(rankingResponse), new Delete<RankingResponseEnumerator>(DeleteRankingEnumeratorAdapter))
            {
            }

            public ActionProbability Current
            {
                get
                {
                    return GetRankingEnumeratorCurrent(this.NativeHandle);
                }
            }

            object System.Collections.IEnumerator.Current => this.Current;

            public bool MoveNext()
            {
                int result;
                if (this.initialState)
                {
                    this.initialState = false;
                    result = RankingEnumeratorInit(this.NativeHandle);
                }
                else
                {
                    result = RankingEnumeratorMoveNext(this.NativeHandle);
                }

                // The contract of result is to return 1 if true, 0 if false.
                return result == 1;
            }

            public void Reset()
            {
                throw new NotSupportedException();
            }
        }
    }
}