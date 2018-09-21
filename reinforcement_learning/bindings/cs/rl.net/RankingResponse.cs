using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;

namespace Rl.Net {
    [StructLayout(LayoutKind.Sequential)]
    internal struct ActionProbability
    {
        public UIntPtr ActionId; // If we expose this publicly, this will not be CLS-compliant
                                 // No idea if that could cause issues going to .NET Core (probably
                                 // not, but this is something we should check.), but having to do
                                 // a conversion on every iteration feels very heavyweight. Can 
                                 // we change this contract to be defined as the signed version of
                                 // the pointer type?
        public float Probability;
    }

    public sealed class RankingResponse: NativeObject<RankingResponse>
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

        // TODO: Once we expose direct manipulation methods on configuration, this can go public
        public RankingResponse() : base(new New<RankingResponse>(CreateRankingResponse), new Delete<RankingResponse>(DeleteRankingResponse))
        {
        }

        public string EventId => GetRankingEventId(this.NativeHandle);

        public string ModelId => GetRankingModelId(this.NativeHandle);

        // TODO: Do we want to keep this consistent with .NET Collection idiom, or with the
        // native RL Library naming (Size)?
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
        public bool TryGetChosenAction(out long action, ApiStatus status = null)
        {
            action = -1;
            UIntPtr chosenAction;
            int result = GetRankingChosenAction(this.NativeHandle, out chosenAction, status.ToNativeHandleOrNullptr());

            if (result != NativeMethods.SuccessStatus)
            {
                return false;
            }

            action = (long)(chosenAction.ToUInt64());
            return true;
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
            private static extern int RankingEnumeratorMoveNext(IntPtr rankingEnumeratorAdapter);
        
            [DllImport("rl.net.native.dll")]
            private static extern ActionProbability GetRankingEnumeratorCurrent(IntPtr rankingEnumeratorAdapter);

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
                // The contract of result is to return 1 if true, 0 if false.
                int result = RankingEnumeratorMoveNext(this.NativeHandle);

                return result == 1;
            }

            public void Reset()
            {
                throw new NotSupportedException();
            }
        }
    }
}