The `windows-2019` image defaults to using the `10.0.22000.0` Windows SDK. This causes a problem when building older versions of Python in vcpkg. This directory contains overlays for each Python version that was unpatched in vcpkg (predated the SDK)

They are just copies of the state of the port at that time in history with the following patch additionally applied: https://github.com/microsoft/vcpkg/pull/20292
