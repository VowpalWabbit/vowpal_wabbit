Deploying Vowpal Wabbit
Nick Nussbaum
9/7/14

The deploy_vw project makes a folder containing vw.exe,spanning tree.exe together with the the Redistributable Visual Studio 2013 c++ runtime.
You can put the folder on a machine which does not have Visual Studio 2013 installed and run the vw.exe  without installing anything.
If your environment has Visual Studio 2013 installed, there's no point in doing  this since the runtime files are already installed on the system.
Th deploy_vw  projects are by default unchecked in the build configuration manager so they aren't built everytime the solution is compiled.

Normally you'd give people the x86 or x64 release versions of the deployment folder
If you're debugging clusters with a remote debugger, you may want to build the debug versions.
These  includes the pdb files as well as the debug versions of the runtime.
These debug runtime versions are not redistributable. You can use them in your machines that have a Visual Studio Licence but can't distribute them to machines without a Visual Studio license.  Check the Microsoft documentation for details on this restriction.

To make the deployment 
In the Build > Configuration Manager menu set the Active Configuration and Active Platform. 
X64 and release would be a common choice.

Select thed deploy_vw project in Solution Explorer

select Build  Menu< Clean deploy_vw
select Build Menu, Rebuild deploy_vw

This will trigger builds of vw and spanning tree if needed and then create the folder vowpal_wabbit\vowpalwabbit\deploy\x64\Release. This folder can be copied to other Windows machines in order to run Vowpal Wabbit.



