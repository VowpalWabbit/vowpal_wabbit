Building VW on Windows -- only 32 bit right now; 64 bit to come

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


    ==> Get pre-built binaries from boostpro -- BUT ONLY 32 BIT BINS ARE AVAILABLE

          http://boostpro.com/download/boost_1_50_setup.exe

          NOTE -- be sure to install binaries for VS 2010, and to check
                  ALL OF THE BOXES on the right hand side! If you get a
                  boost link error, this is the most likely culprit!

          ALSO NOTE -- you'll need to install more information

(2) Pick a base directory for sources -- I'll use c:\src\vw

(3) Download zlib from here:

  http://zlib.net/zlib127.zip

(4) Unzip to %ROOT% -- on my machine, this lands in c:\src\vw\zlib-1.2.7.

  (This must have the correct relative path for builds to work)

(5) Build zlib

    (a) Start a new CMD window
    (b) Run "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" to set build variables
    (c) Go to the %ROOT%\zlib-1.2.7\zlib-1.2.7\contrib\vstudio\vc10 directory (for me, c:\src\vw\zlib-1.2.7zlib-1.2.7\contrib\vstudio\vc10)
    (d) Patch up the zlibstat.vcxproj to correctly use DLL versions of the runtime for 32bit platforms (ugh):

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
    (b) "git clone https://github.com/chrisquirk/vowpal_wabbit.git"

(7) Open %ROOT%\vowpal_wabbit\vowpalwabbit\vw.sln in Visual Studio 2010 and hit Build.

(8) Build. Binaries will be in one of these four directories, based on whether you built DEBUG or RELEASE bits and whether you are building x64 or Win32.

  %ROOT%\vowpal_wabbit\vowpalwabbit\Debug\vw.exe
  %ROOT%\vowpal_wabbit\vowpalwabbit\Release\vw.exe
  %ROOT%\vowpal_wabbit\vowpalwabbit\x64\Debug\vw.exe
  %ROOT%\vowpal_wabbit\vowpalwabbit\x64\Release\vw.exe


