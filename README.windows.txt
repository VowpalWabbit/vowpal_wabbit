Building VW on Windows

8/15/2012, Chris Quirk <chrisq@microsoft.com>

You need Visual Studio 2010

(1) Install boost 1.50. There are several options available.

  (This must have the correct absolute path for builds to work)

    ==> Get pre-built binaries from someone else.

      (a) Download my pre-built boost-1.50-bins.zip from SkyDrive:

            http://sdrv.ms/RXV5gt

      (b) Unzip in the root of your C: drive, so you should have c:\boost\x86 and c:\boost\x64 directories.


    ==> Build boost from scratch:

      (a) Download boost_1_50_0.zip from here http://sourceforge.net/projects/boost/files/boost/1.50.0/
      (b) Unzip to someplace convenient (I use c:\src)
      (c) Open a new command window
      (d) Run "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" to set build variables
      (e) cd to the directory where you unzipped boost, and run "bootstrap.bat"
      (f) Run "mkdir c:\boost"
      (g) Run "mkdir c:\boost\x86"
      (h) Run "mkdir c:\boost\x64"
      (i) cd c:\src
      (j) bootstrap.bat
      (k) Run "b2 --prefix=c:\boost\x86 --build-dir=x86 --toolset=msvc install --with-program_options" (I add " -j 16" to the end to run up to 16 procs at once.)
      (l) Run "b2 --prefix=c:\boost\x64 --build-dir=x64 --toolset=msvc address-model=64 install --with-program_options"

if you have multiple Visual Studios installed (vs2012 and vs2010) explicitly specify the toolset version
	  toolset=msvc-10.0
	 
	  
    ==> Get pre-built binaries from boostpro -- BUT ONLY 32 BIT BINS ARE AVAILABLE

          http://boostpro.com/download/boost_1_50_setup.exe

          NOTE -- be sure to install binaries for VS 2010, and to check
                  ALL OF THE BOXES on the right hand side! If you get a
                  boost link error, this is the most likely culprit!

          ALSO NOTE -- you'll need to install more information

(2) Pick a base directory for sources -- I'll use c:\src\vw

(3) Download zlib from here:

  http://zlib.net/zlib128.zip

(4) Unzip to %ROOT% -- on my machine, this lands in c:\src\vw\zlib-1.2.8.

  (This must have the correct relative path for builds to work)

(5) Build zlib

    (a) Start a new CMD window
    (b) Run "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" to set build variables
    (c) Go to the %ROOT%\zlib-1.2.8\zlib-1.2.8\contrib\vstudio\vc10 directory (for me, c:\src\vw\zlib-1.2.8\zlib-1.2.8\contrib\vstudio\vc10)
    (d) Patch up the zlibstat.vcxproj to correctly use DLL versions of the runtime for 32bit platforms (ugh).  This requires editing lines 167, 194, 222:

***************
*** 164,170 ****
        <PreprocessorDefinitions>WIN32;ZLIB_WINAPI;_CRT_NONSTDC_NO_DEPRECATE;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_WARNINGS;%(PreprocessorDefinitio
ns)</PreprocessorDefinitions>
        <ExceptionHandling>
        </ExceptionHandling>
!       <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
        <BufferSecurityCheck>false</BufferSecurityCheck>
        <PrecompiledHeaderOutputFile>$(IntDir)zlibstat.pch</PrecompiledHeaderOutputFile>
        <AssemblerListingLocation>$(IntDir)</AssemblerListingLocation>
--- 164,170 ----
        <PreprocessorDefinitions>WIN32;ZLIB_WINAPI;_CRT_NONSTDC_NO_DEPRECATE;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_WARNINGS;%(PreprocessorDefinitio
ns)</PreprocessorDefinitions>
        <ExceptionHandling>
        </ExceptionHandling>
!       <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>
        <BufferSecurityCheck>false</BufferSecurityCheck>
        <PrecompiledHeaderOutputFile>$(IntDir)zlibstat.pch</PrecompiledHeaderOutputFile>
        <AssemblerListingLocation>$(IntDir)</AssemblerListingLocation>
***************
*** 191,197 ****
        <StringPooling>true</StringPooling>
        <ExceptionHandling>
        </ExceptionHandling>
!       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
        <BufferSecurityCheck>false</BufferSecurityCheck>
        <FunctionLevelLinking>true</FunctionLevelLinking>
        <PrecompiledHeaderOutputFile>$(IntDir)zlibstat.pch</PrecompiledHeaderOutputFile>
--- 191,197 ----
        <StringPooling>true</StringPooling>
        <ExceptionHandling>
        </ExceptionHandling>
!       <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
        <BufferSecurityCheck>false</BufferSecurityCheck>
        <FunctionLevelLinking>true</FunctionLevelLinking>
        <PrecompiledHeaderOutputFile>$(IntDir)zlibstat.pch</PrecompiledHeaderOutputFile>
***************
*** 219,225 ****
        <StringPooling>true</StringPooling>
        <ExceptionHandling>
        </ExceptionHandling>
!       <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
        <BufferSecurityCheck>false</BufferSecurityCheck>
        <FunctionLevelLinking>true</FunctionLevelLinking>
        <PrecompiledHeaderOutputFile>$(IntDir)zlibstat.pch</PrecompiledHeaderOutputFile>
--- 219,225 ----
        <StringPooling>true</StringPooling>
        <ExceptionHandling>
        </ExceptionHandling>
!       <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
        <BufferSecurityCheck>false</BufferSecurityCheck>
        <FunctionLevelLinking>true</FunctionLevelLinking>
        <PrecompiledHeaderOutputFile>$(IntDir)zlibstat.pch</PrecompiledHeaderOutputFile>
***************

    (e) Run the following four commands (can skip the last two if you only want 32bit binaries)

        "msbuild /p:Configuration=Debug;Platform=Win32 zlibstat.vcxproj"
        "msbuild /p:Configuration=Release;Platform=Win32 zlibvc.vcxproj"
        "msbuild /p:Configuration=Release;Platform=Win32 zlibstat.vcxproj"
        "msbuild /p:Configuration=Debug;Platform=x64 zlibstat.vcxproj"
        "msbuild /p:Configuration=Release;Platform=x64 zlibvc.vcxproj"
        "msbuild /p:Configuration=Release;Platform=x64 zlibstat.vcxproj"

(6) Get a copy of VW in %ROOT%. I ran "cd \src\vw" and "git clone http

    (a) Change to root (for me, "cd \src\vw")
    (b) "git clone https://github.com/JohnLangford/vowpal_wabbit.git"

(7) Open %ROOT%\vowpal_wabbit\vowpalwabbit\vw.sln in Visual Studio 2010 and hit Build.

(8) Build. Binaries will be in one of these four directories, based on whether you built DEBUG or RELEASE bits and whether you are building x64 or Win32.

  %ROOT%\vowpal_wabbit\vowpalwabbit\Debug\vw.exe
  %ROOT%\vowpal_wabbit\vowpalwabbit\Release\vw.exe
  %ROOT%\vowpal_wabbit\vowpalwabbit\x64\Debug\vw.exe
  %ROOT%\vowpal_wabbit\vowpalwabbit\x64\Release\vw.exe


**************************************************************************************************************
**************************************************************************************************************
**************************************************************************************************************
Notes for building VW under Visual Studio 2013 on Windows 8.1
Nick Nussbaum nickn@seanet.com
8/16/2014

**************************************************************************************************************
(1) Get Tools
You'll need a Visual Studio 2013 installed that includes c# and c++
You'll also need the Windows SDK which you can download from Microsoft at
	http://msdn.microsoft.com/en-us/windows/desktop/bg162891.aspx


There's a patch for zlib to make it work.
There also some changes to vowpal wabbit  Details are at the last section of this file

It's  handy to have a bash shell to run patch and git
You can use a git bash shell fron the https://windows.github.com/ if you don't have it already.
Or you can just edit the changes using notepad to read the files. Git Patching seemed to have some problems with the files.

**************************************************************************************************************
(2) open a copy various command shells

	(a)	Open an x86 command shell: run the Visual Studio 2013 Tools /  VS2013 x86 Native Tools Command Prompt
			or run:	 cmd.exe /k "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
	(b)	Open an x64 command shell: run the Visual Studio 2013 Tools / VS2013 x64 Cross Tools Command Prompt
			or run:   cmd.exe /k "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64
	(c)	Open the Git bash shell "C:\Program Files (x86)\Git\bin\sh.exe" --login -i
			or some other bash shell

**************************************************************************************************************
(3) Setup Directories

I use c:\src\vw as my %ROOT% directory; You could use another directory 
boost, vowpal_wabbit, and zlib-1.2.8 are directories inside that directory

        (a) mkdir c:\src
        (b) mkdir c:\src\vw

**************************************************************************************************************
(4) Get Vowpal Wabbit 

    (a) In a  command shell to %ROOT% : "cd c:\src\vw"
    (b) run "git clone https://github.com/JohnLangford/vowpal_wabbit.git"
	details of the changes are in bottom of this file.

**************************************************************************************************************
(5) Build zlib with Visual Studio 2013

	(a)Get the zlib 1.28.0 file from   http://zlib.net/zlib128.zip
	(b) unzip zlib-1.2.8.zip into the c:\src\vw\zlib-1.2.8  

	use contrib/vstudio/vc11 since there is no contrib/vstudio/vc12 as yet


	(c) from a bash shell cd  /c/src/vw

	  patch --dry-run -po --directory=zlib-1.2.8 --input=../vowpal_wabbit/zlibpatch.txt -F3
		check output looks good then
	  patch  -po --directory=zlib-1.2.8 --input=../vowpal_wabbit/zlibpatch.txt -F3

	(d) Build the zlib libararies using by either of

	Launch Visual Studio 2013
	Open the solution %ROOT%/zlib-1.2.8\contrib\vstudio\vc11\zlibvc.sln
	Batch build the configurations you want of x86 and x64 debut and release

    	(e) or from your Visual Studio Command shell run the following four commands (can skip the last two if you only want 32bit binaries)

        "msbuild /p:Configuration=Debug;Platform=Win32 zlibstat.vcxproj"
        "msbuild /p:Configuration=Release;Platform=Win32 zlibvc.vcxproj"
        "msbuild /p:Configuration=Release;Platform=Win32 zlibstat.vcxproj"
        "msbuild /p:Configuration=Debug;Platform=x64 zlibstat.vcxproj"
        "msbuild /p:Configuration=Release;Platform=x64 zlibvc.vcxproj"
        "msbuild /p:Configuration=Release;Platform=x64 zlibstat.vcxproj"

	Ignore the warnings about Platform 'Itanium' referenced in the project file  since Itanium is no longer supported in Visual Studio 2013

**************************************************************************************************************
(6) Building Boost

I build boost in c:\boost
You can use another directory but you will have to modify the vw solution and project variables


	Get boost from http://www.boost.org/users/history/version_1_56_0.html
			

	   open a  Windows command shell
	  (a) mkdir c:\boost 
			
      (b) Download boost_1_56_0.zip from http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.zip/download
	  (c) Unzip it which creates the directory boost_1_56_0
      (d) mkdir c:\boost\x86
	  (e) mkdir c:\boost\x64
 	  
build the x86 binaries
	  (f) run "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat x86"
	  (g) mkdir c:\boost\x86
	  (h) cd c:\boost\boost_1_56_0
	  (i) run "bootstrap.bat"
	  (j) run "b2 --prefix=c:\boost\x86 --build-dir=x86 --toolset=msvc-12.0 address-model=32 install --with-program_options" 
			(You can add " -j 16" to the end to run up to 16 procs at once.)

	
build the x64 binaries
	  (k) mkdir c:\boost\x64	
	  (l) run "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"  x86_amd64"
	  (m) run  "bootstrap.bat"
	  (n) run ".\b2 --prefix=c:\boost\x64 --build-dir=x64 --toolset=msvc-12.0 address-model=64 install --with-program_options"

	  
	  
**************************************************************************************************************
(7) Build Vowpal Wabbit 


	Open %ROOT%\vowpal_wabbit\vowpalwabbit\vw.sln in Visual Studio 2010 and run  rebuild or batch build

	Binaries will be in one of these four directories, based on whether you built DEBUG or RELEASE bits and whether you are building x64 or Win32.

  	%ROOT%\vowpal_wabbit\vowpalwabbit\Debug\vw.exe
  	%ROOT%\vowpal_wabbit\vowpalwabbit\Release\vw.exe
  	%ROOT%\vowpal_wabbit\vowpalwabbit\x64\Debug\vw.exe
  	%ROOT%\vowpal_wabbit\vowpalwabbit\x64\Release\vw.exe

**************************************************************************************************************
(8) Test
	There's a new test batch file that runs a quick test on all four configutations
	(a) go to a windows command shell
	(a) cd c:\src\vw\test
 	(b) run test\test_2_winvw.bat


**************************************************************************************************************
(9) Appendix The Gory Details of the patches and VW upgrades

(a) misc files
	   adds this content to Windows.ReadMee
	   adds the  file vowpal_wabbit\zlibpatch.txt a patch for xlib
	   adds the file test\test_2_winvw.bat a simple test of x86 and x64 training and prediction


(b) Changes to Zlib
This Zlib patch includes the following fixes;

Convert to VS2013 solution

The fix in the prior section to correctly use DLL versions of the runtime for 32bit platforms
Changes to use only two fields in zlibvc.def VERSTION 
-	VERSION		1.2.8
+	VERSION		1.28
since otherwise the compiler will complain about more than 2 fields and ignore them.

add /safeseh to the x86 assembler so Visual Studio will not generate an error "unable to generate SAFESSH image"
This is not need for x64 since it happens by default


In the properties sheet for zlibvc

The pre build command line for x64 release should be fixed
-cd ..\..\contrib\masmx64
+cd ..\..\masmx64

 Code generation: Runtime Library for windows release  set to  Multi-threaded DLL (/MD) not /MT for zlibvc and zlibstat
Otherwise VS13 will complain about multiple runtime specifications.


(c) Change to Boost 1.56.0

(d) Changes to VowpalWabbit
	
	changes vw projects and solutions to run under VS2013 rather than Visual Studio 2012
	change vw projects to redefine $(BoostIncludeDir) to refer to Boost 1.56.0
	change vw projects to define $(BoostLibDir) to refer to Boost 1.56.0

	vowpalwabbit/vw_static.vcxproj
		Define $(IncludePath) 
		change 	$(ZlibDir) to use \contrib\vstudio\vc11 rather than vc10
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
		Note building the anycpu solution has problems with cs_test
	

      	vowpalwabbit/vw.sln
		change configurations to use Debug|x86 from Debug|AnyCpu 

      c_test/c_test.vcxproj
		change to VS 12
		change configurations to use Debug|x86 from Debug|AnyCpu 
		change cs_test to use x86 and x64 rather than anycpu


