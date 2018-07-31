Originally by Chris Quirk <chrisq@microsoft.com>
**************************************************************************************************************
Notes for building VW under Visual Studio 2013 on Windows 8.1
9/02/2014 Nick Nussbaum nickn@seanet.com

Replace source dependencies with Nuget
04/29/2015 Sharat Chikkerur sharat.chikkerur@gmail.com

Added ANTLR based unit test
10/2/2015 Markus Cozowicz marcozo@microsoft.com

Notes for building VW under Visual Studio 2015 on Windows 10
07/30/2018 Mandy Shieh meshieh@microsoft.com

**************************************************************************************************************
(1) Get Tools
You'll need Visual Studio 2015 installed with c# and c++

You should install Visual Studio 2015 Update 3 (or Visual Studio 2013 Update 5). If you have a later version
but not 2015, you'll have to uninstallthat first to successfully install VS2015.
    https://my.visualstudio.com/Downloads?q=visual%20studio%202015&wt.mc_id=o~msft~vscom~older-downloads

You'll need the Windows SDK which can be downloaded from Microsoft at
    https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk

You'll need Nuget integration with Visual Studio
    http://docs.nuget.org/consume

You'll need Java to run unit tests
    http://www.oracle.com/technetwork/java/javase/downloads/jre8-downloads-2133155.html

You'll need Vcpkg, a library manager for c++. Clone and build vcpkg following instructions:
    https://github.com/Microsoft/vcpkg

**************************************************************************************************************
(2) Get Packages
With Vcpkg, you can download the necessary libraries. There are two for Vowpal Wabbit: cpprestsdk and boost,
but you only need to install cpprestsdk, which includes boost. Installing both will confuse the build system 
and cause errors.

In Powershell, run command
    ./vcpkg integrate install        (This will give your builds access to installed packages)
    ./vcpkg install cpprestsdk cpprestsdk:x64-windows
This may take around 20 minutes. If you accidentally installed boost as well, the quickest way is to delete the
vcpkg repo and start over.

**************************************************************************************************************
(3) Open a copy various command shells

    (a) Open an x86 command shell: run the Visual Studio 2013 Tools /  VS2013 x86 Native Tools Command Prompt
        or 
        cmd.exe /k "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
    (b) Open an x64 command shell: run the Visual Studio 2013 Tools / VS2013 x64 Cross Tools Command Prompt
        or 
        cmd.exe /k "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64
    (c) Open the Git bash shell
        "C:\Program Files (x86)\Git\bin\sh.exe" --login -i
        or some other bash shell
    (d) If none of these work, try locating the command line tools based on directions here: 
        https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs

**************************************************************************************************************
(4) Setup Directories

I use c:\src\vw as my %ROOT% directory; 

    (a) mkdir c:\src
    (b) mkdir c:\src\vw

**************************************************************************************************************
(5) Get Vowpal Wabbit 

    (a) In a  command shell to %ROOT% : "cd c:\src\vw"
    (b) run "git clone https://github.com/JohnLangford/vowpal_wabbit.git"
        details of the changes are in bottom of this file.

**************************************************************************************************************
(6) Install Visual Studio Project Extensions
At this point, if you try to open vw.sln, some projects may show as unable to load. This is because Visual
Studio is missing project extensions. There are two that are necessary here:

    (a) To load azure_service.ccproj, you'll need Azure Tools for VS2015:
            https://docs.microsoft.com/en-us/dotnet/azure/dotnet-sdk-vs2015-install?view=azure-dotnet
        You may need to open azure_service.ccproj and edit <CloudExtensionDir> to the same location and 
        version as your installation. For me (July 2018) this was changing the version of
        $(MSBuildExtensionsPath)\Microsoft\VisualStudio\v$(VisualStudioVersion)\Windows Azure Tools\2.8\ to 
        $(MSBuildExtensionsPath)\Microsoft\VisualStudio\v$(VisualStudioVersion)\Windows Azure Tools\2.9\
    (b) To load *.wixproj, you'll need the WiX extensions. In VS2015, go to Tools>Extensions and Updates>
        Online, and search for WiX. You’ll need both WiX Toolset Build Tools, which you can download and run
        from a browser, and WiX Toolset VS2015 Extension​​​​​, which can be installed directly in Visual Studio.
        
**************************************************************************************************************
(7) Restore nugets

    (a) In a  command shell to %ROOT%\vowpalwabbit\vowpalwabbit : "cd c:\src\vw\vowpalwabbit\vowpalwabbit" 
    (b) run ".nuget\nuget restore vw.sln"
        This will restore the ANTLR nuget which is needed before Visual Studio loads the solution.        

**************************************************************************************************************
(8) Build Vowpal Wabbit 

    (a) Using Visual Studio
        Open %ROOT%\vowpal_wabbit\vowpalwabbit\vw.sln in Visual Studio 2015
        Set startup project as vw (or the test project)
             
        Select x64 platform (Configuration Manager \ Active solution platfrom)
        Select x64 as test platform (Test \ Test settings \ Default Processor Architecture)

        If you installed VS2015 when a later version was already present, you may need to retarget all the c++
        projects back to Platform Toolset v140. Right click on each project>Properties>Configuration Properties>
        General>Platform Toolset: Visual Studio 2015(v140)

        Build solution by clicking Build>Rebuild Solution, or run batch build.
        Binaries will be in one of these four directories, based on whether you built DEBUG or RELEASE bits and
        whether you are building x64.
            %ROOT%\vowpal_wabbit\vowpalwabbit\x64\Debug\vw.exe
            %ROOT%\vowpal_wabbit\vowpalwabbit\x64\Release\vw.exe
        Missing nugets will be installed during the build.

    (b) Using command line (available configurations are "Release" and "Debug". Available platforms are "x64" and "Win32")
        run>msbuild /p:Configuration="Release" /p:Platform="x64" vw.sln
        
        Note: If you failed to do so before opening the solution, the cs_unittest
        project is in a "not loaded" state. After executing the above you'll have to
        hit "Reload" (Project / Context Menu) in Visual Studio.

**************************************************************************************************************
(9) Test
        There's a new test batch file that runs a quick test on all four configurations
        (a) go to a windows command shell
        (b) cd c:\src\vw\test
        (c) run test\test_2_winvw.bat

**************************************************************************************************************
(10) Appendix: The Gory Details of the patch and VW upgrades

    (a) misc files
        adds this content to this file ReadMe.Windows.txt
        adds the  file vowpal_wabbit\zlibpatch.txt a patch for xlib
        adds the file test\test_2_winvw.bat a simple test of x86 and x64 training and prediction


    (b) Changes to Zlib
    This Zlib patch includes the following fixes;

    Convert to Visual Studio 2013 solution

    The fix in the prior section to correctly use DLL versions of the runtime for 32bit platforms
    Changes to use only two fields in zlibvc.def VERSTION 
    -    VERSION        1.2.8
    +    VERSION        1.28
    since otherwise the compiler will complain about more than 2 fields and ignore them.

    add /safeseh to the x86 assembler so Visual Studio will not generate an error "unable to generate SAFESSH image"
    This is not need for x64 since it happens by default


    In the properties sheet for zlibvc

    The pre build command line for x64 release should be fixed
    -cd ..\..\contrib\masmx64
    +cd ..\..\masmx64

    Code generation: Runtime Library for windows release  set to  Multi-threaded DLL (/MD) not /MT for zlibvc and zlibstat
    Otherwise VS13 will complain about multiple runtime specification while trying to autolink


    (c) Change to Boost 1.56.0

    (d) Changes to VowpalWabbit
            
        changes vw projects and solutions to run under Visual Studio 2013 rather than Visual Studio 2012
        change vw projects to redefine $(BoostIncludeDir) to refer to Boost 1.56.0
        change vw projects to define $(BoostLibDir) to refer to Boost 1.56.0

        vowpalwabbit/vw_static.vcxproj
            Define $(IncludePath) 
            change         $(ZlibDir) to use \contrib\vstudio\vc11 rather than vc10
            change x64 version DebugInformationFormat  to use "ProgramDatabase" and not the invalid "EditAndContinue"

            change IntermediateFolderPath to include ProjectName so two projects aren't trying to build in the same folder
            add searn_multiclasstask.cc to the project
            change include path to all use macros $(VC_IncludePath);$(WindowsSDK_IncludePath)
            change additional dependencies to use  $(SolutionDir)$(PlatformShortName)\$(Configuration)\vw_static.lib

            adds a reference to the WindowsSDKDir Include\um
            change vw_static properties for debug 64bit to /Zi from /Zl to shut up some warnings.
            change the vw and static_vw to use n intermediate directories that appends the $(ProjectName). 
            this avoid various conflicts and warnings caused by dumping into the same directory.
            change link build copies to use PlatformShortName rather than PlatformName to use x86 rather than Win32
            Change the anycpu confuuration for problems with cs_test
            

        vowpalwabbit/vw.sln
            change configurations to use Debug|x86 from Debug|AnyCpu 

        c_test/c_test.vcxproj
            change to VS 12
            change configurations to use Debug|x86 from Debug|AnyCpu 
            change cs_test to use x86 and x64 rather than anycpu
            change test file specs to reference the .../../... test directory 
