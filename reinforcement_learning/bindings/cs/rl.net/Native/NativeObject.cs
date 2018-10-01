using System;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Threading;

using System.Diagnostics;

namespace Rl.Net.Native
{
    public delegate IntPtr New<THandle>() where THandle : NativeObject<THandle>;
    public delegate void Delete<THandle>(IntPtr pointer) where THandle : NativeObject<THandle>;

    [SecurityPermission(SecurityAction.InheritanceDemand, UnmanagedCode = true)]
    [SecurityPermission(SecurityAction.Demand, UnmanagedCode = true)]
    public abstract class NativeObject<THandle> : SafeHandle //SafeHandleZeroOrMinusOneIsInvalid
        where THandle : NativeObject<THandle>
    {
        private readonly Delete<THandle> operatorDelete;

        protected NativeObject(New<THandle> operatorNew, Delete<THandle> operatorDelete)
            : base(operatorNew(), ownsHandle: true)
        {
            this.operatorDelete = operatorDelete;

            // TODO: Check nulls in Debug
            Debug.WriteLine($"New object at at {this.handle.ToInt64():x}");
        }

        internal IntPtr NativeHandle
        {
            get
            {
                return this.handle;
            }
        }

        override public bool IsInvalid
        {
            get
            {
                return this.handle == IntPtr.Zero;
            }
        }

        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        override protected bool ReleaseHandle()
        {
            // In NetFx, Interlocked.Exchange(IntPtr...) is marked with ReliabilityContract(), so it is
            // safe to call here. In NetCore, Interlocked.Exchange(IntPtr...) is marked with Intrinsic().
            // so it *should* be safe.
            IntPtr localHandle = Interlocked.Exchange(ref this.handle, IntPtr.Zero);
            if (localHandle == IntPtr.Zero)
            {
                // The SafeHandle contract is supposed to guarantee that ReleaseHandle() will only be
                // called once, and only if the handle is valid (in our case != IntPtr.Zero). Failures
                // here can be caused by odd timing issues under the hood, so raise a Managed Debugging
                // Assistant event (in VS with DebuggerAttached) by returning false here.
                return false;
            }

            // TODO: Check nulls in Debug
            Debug.WriteLine($"Deleting object at at {localHandle.ToInt64():x}");

            // Here, we must obey all rules for constrained execution regions.
            operatorDelete(localHandle);
            
            return true;
        }
    }
}