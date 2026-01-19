# Configure multiprocessing to use forkserver instead of fork on Linux.
# This is needed because pybind11 modules don't survive fork() cleanly.
# The forkserver method starts a clean server process before any modules are
# imported, then forks from that server. This avoids the pybind11 fork issues
# while being faster than spawn (which starts a new Python interpreter each time).
#
# This must be set before any multiprocessing pools are created, which is why
# it's in conftest.py (loaded early by pytest before test modules).
import multiprocessing
import sys

if sys.platform == 'linux':
    try:
        multiprocessing.set_start_method('forkserver')
    except RuntimeError:
        # Already set, ignore
        pass

