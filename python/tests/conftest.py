# Configure multiprocessing to use forkserver instead of fork on Python 3.11+.
# This is needed because pybind11 modules don't survive fork() cleanly
# on some Python versions. The forkserver method starts a clean server
# process before any modules are imported.
#
# We only do this on Python 3.11+ where the issue has been observed.
# On Python 3.10, fork works fine and is faster.
import multiprocessing
import sys

if sys.platform == 'linux' and sys.version_info >= (3, 11):
    try:
        multiprocessing.set_start_method('forkserver')
    except RuntimeError:
        # Already set, ignore
        pass

