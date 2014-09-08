The deploy_vw project  makes a folder containing vw.exe,spanning tree.exe together with the the Redistributable Visual Studio 2013 c++ runtimes needed to use  .exe
You can put the folder on a machine which does not have Visual Studio 2013 installed and run the vw.exe  without installing anything.
If your environment has Visual Studio 2013 installed, there's no point in doing  this since the runtime files are already installed on the system.
That's why the deployer  projects are by default unchecked in the build configuration manager.

Normally you'd give people the x86 or x64 release versions.
If you're debugging clusters with a remote debugger, you may want to build the debug versions.
This is larger since it includes the pdb files as well as the debug versions of the runtime.
These debug versions are not redistributable. You can use them in your machines that have a Visual Studio Licence but can't hand it out to people without Visual Studio..

To make the deployment 
In the Build > Configuration Manager menu set the Active Configuration and Active Platform. 
X64 and release would be a common choice.

Select thed deployer project in Solution Explorer

In the Build  Menu run Clean deploy
In the Build Menu run Rebuild deploy

This should trigger builds of vw and spanning tree if needed and then build the volder

The folder called  vowpal_wabbit\vowpalwabbit\deploy\x64\Release 



