Building VW on Windows

Originally by Chris Quirk <chrisq@microsoft.com>
**************************************************************************************************************
Notes for building VW under Visual Studio 2013 on Windows 8.1
9/02/2014 Nick Nussbaum nickn@seanet.com


**************************************************************************************************************
(1) Get Tools
You'll need a Visual Studio 2013 installed that includes c# and c++
You'll also need the Windows SDK which you can download from Microsoft at
	http://msdn.microsoft.com/en-us/windows/desktop/bg162891.aspx


There's a patch for zlib to make it work.
There also some changes to Vowpal Wabbit in this commit.
Details are at the last section of this file

It's  handy to have a bash shell to run git
You can use a git bash shell fron the https://windows.github.com/ if you don't have it already.
Or you can just edit the changes using notepad to read the files. 

There are end of line problems with patching with git patch.
I used the GnuWin32 patch package binaries from
http://gnuwin32.sourceforge.net/packages/patch.htm which will run in a dos batch file.
This seems to be able to deal with patching without damaging the windows <CR><LF> pairs.

**************************************************************************************************************
(2) open a copy various command shells

	(a)	Open an x86 command shell: run the Visual Studio 2013 Tools /  VS2013 x86 Native Tools Command Prompt
			or 
			cmd.exe /k "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
	(b)	Open an x64 command shell: run the Visual Studio 2013 Tools / VS2013 x64 Cross Tools Command Prompt
			or 
			cmd.exe /k "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64
	(c)	Open the Git bash shell
		"C:\Program Files (x86)\Git\bin\sh.exe" --login -i
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

	The patched version of the sources for zlib are up on https://github.com/nicknussbaum/zlibpatched.git
	They can also be made as follows.
	

	(a)Get the zlib 1.28.0 file from   http://zlib.net/zlib128.zip
	(b) unzip zlib-1.2.8.zip into the c:\src\vw\zlib-1.2.8  

	use contrib/vstudio/vc11 since there is no contrib/vstudio/vc12 as yet
	
	(c)Get the GnuWin32 Patch Utility http://gnuwin32.sourceforge.net/packages/patch.htm patch.exe
	and simply put it in the c:\src\vw

	(d)Unzip the zlibpatch.zip file and place the contained zlibpatch.txt file in vowpa_wabbit 



	(e) From a dos command shell run as administrator
	
	  patch --dry-run -p0 --directory=zlib-1.2.8 --input=../vowpal_wabbit/zlibpatch.txt -F3
		check output messages looks good then
	  patch  -p0 --directory=zlib-1.2.8 --input=../vowpal_wabbit/zlibpatch.txt -F3
	  
	  
	  
	  
	  

	 From the patched sources build the zlib libararies by either of the following steps. 

	Launch Visual Studio 2013
	Open the solution %ROOT%/zlib-1.2.8\contrib\vstudio\vc11\zlibvc.sln
	Batch build the configurations you want of x86 and x64 debut and release

    	 or from your Visual Studio Command shell 
	     cd c:\src\vw\zlib-1.2.0\contrib\vstudio\vc11
		run the following commands (can skip the last four if you only want 32bit binaries)

        "msbuild /p:Configuration=Debug;Platform=Win32 zlibstat.vcxproj"
        "msbuild /p:Configuration=Release;Platform=Win32 zlibvc.vcxproj"
        "msbuild /p:Configuration=Release;Platform=Win32 zlibstat.vcxproj"
        "msbuild /p:Configuration=Debug;Platform=x64 zlibstat.vcxproj"
        "msbuild /p:Configuration=Release;Platform=x64 zlibvc.vcxproj"
        "msbuild /p:Configuration=Release;Platform=x64 zlibstat.vcxproj"

	Ignore the warnings about Platform 'Itanium' referenced in the project file  since Itanium is no longer supported 
	
	
	

**************************************************************************************************************
(6) Building Boost

I build boost in c:\boost with the sources in a subdirectory
If you use another directory modify the vw solution and project macro definitions for BoostIncludeDir and BoostLibDir


	Get boost from http://www.boost.org/users/history/version_1_56_0.html
			

	   open a  Windows command shell
	  (a) mkdir c:\boost 
			
      (b) Download boost_1_56_0.zip from http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.zip/download
	  (c) Unzip it which creates the directory boost_1_56_0
      (d) mkdir c:\boost\x86
	  (e) mkdir c:\boost\x64
 	  
build the x86 binaries
	  (f)"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat x86"
	  (g) mkdir c:\boost\x86
	  (h) cd c:\boost\boost_1_56_0
	  (i) "bootstrap.bat"
	  (j) "b2 --prefix=c:\boost\x86 --build-dir=x86 --toolset=msvc-12.0 address-model=32 install --with-program_options" 
			(You can add " -j 16" to the end to run up to 16 processors at once.)

	
build the x64 binaries
	  (k) mkdir c:\boost\x64	
	  (l) "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"  x86_amd64"
	  (m) "bootstrap.bat"
	  (n) ".\b2 --prefix=c:\boost\x64 --build-dir=x64 --toolset=msvc-12.0 address-model=64 install --with-program_options"

	  
	  
**************************************************************************************************************
(7) Build Vowpal Wabbit 


	Open %ROOT%\vowpal_wabbit\vowpalwabbit\vw.sln in Visual Studio 2013
	Set startup project as vw (or the test project)
	run  build>rebuild solution
		or run  batch build

	Binaries will be in one of these four directories, based on whether you built DEBUG or RELEASE bits and whether you are building x64 or Win32.

		%ROOT%\vowpal_wabbit\vowpalwabbit\x86\Debug\vw.exe
		%ROOT%\vowpal_wabbit\vowpalwabbit\x86\Release\vw.exe
		%ROOT%\vowpal_wabbit\vowpalwabbit\x64\Debug\vw.exe
		%ROOT%\vowpal_wabbit\vowpalwabbit\x64\Release\vw.exe

**************************************************************************************************************
(8) Test
	There's a new test batch file that runs a quick test on all four configurations
	(a) go to a windows command shell
	(a) cd c:\src\vw\test
 	(b) run test\test_2_winvw.bat


**************************************************************************************************************
(9) Appendix: The Gory Details of the patch and VW upgrades

(a) misc files
	   adds this content to this file ReadMe.Windows.txt
	   adds the  file vowpal_wabbit\zlibpatch.txt a patch for xlib
	   adds the file test\test_2_winvw.bat a simple test of x86 and x64 training and prediction


(b) Changes to Zlib
This Zlib patch includes the following fixes;

Convert to Visual Studio 2013 solution

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
Otherwise VS13 will complain about multiple runtime specification while trying to autolink


(c) Change to Boost 1.56.0

(d) Changes to VowpalWabbit
	
	changes vw projects and solutions to run under Visual Studio 2013 rather than Visual Studio 2012
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
		Change the anycpu confuuration for problems with cs_test
	

     vowpalwabbit/vw.sln
		change configurations to use Debug|x86 from Debug|AnyCpu 

     c_test/c_test.vcxproj
		change to VS 12
		change configurations to use Debug|x86 from Debug|AnyCpu 
		change cs_test to use x86 and x64 rather than anycpu
		change test file specs to reference the .../../... test directory 


