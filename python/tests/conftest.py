# Configure multiprocessing to use forkserver instead of fork on Linux.
# This is needed because pybind11 modules don't survive fork() cleanly.
# The forkserver method starts a clean server process before any modules are
# imported, then forks from that server. This avoids the pybind11 fork issues
# while being faster than spawn (which starts a new Python interpreter each time).
#
# We use force=True because some pytest plugins or the environment may have
# already initialized multiprocessing with fork before conftest.py loads.
import multiprocessing
import sys
import os

if sys.platform == 'linux':
    try:
        multiprocessing.set_start_method('forkserver', force=True)
    except RuntimeError:
        # In case force=True still fails for some reason
        pass

# Diagnostic: Log multiprocessing start method
_mp_method = multiprocessing.get_start_method(allow_none=True)
print(f"[conftest] multiprocessing start method: {_mp_method}", flush=True)
print(f"[conftest] Python version: {sys.version}", flush=True)
print(f"[conftest] Platform: {sys.platform}", flush=True)


def pytest_runtest_logstart(nodeid, location):
    """Log when each test starts - helps identify hanging tests."""
    print(f"\n[TEST START] {nodeid}", flush=True)


def pytest_runtest_logfinish(nodeid, location):
    """Log when each test finishes."""
    print(f"[TEST END] {nodeid}", flush=True)

