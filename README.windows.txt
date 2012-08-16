Building VW on Windows -- only 32 bit right now; 64 bit to come

8/14/2012, Chris Quirk <chrisq@microsoft.com>

You need Visual Studio 2010

(1) Install boost 1.50 from boostpro (only 32bit, unfortunately).

  http://boostpro.com/download/boost_1_50_setup.exe

(2) Pick a base directory for sources -- I'll use c:\src\vw

(3) Download zlib from here:

  http://zlib.net/zlib127.zip

(4) Unzip to %ROOT% -- on my machine, this lands in c:\src\vw\zlib-1.2.7.

  (This must have the correct relative

(5) Build zlib

    (a) Start a new CMD window
    (b) Run "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" to set build variables
    (c) Go to the %ROOT%\zlib-1.2.7 directory (for me, c:\src\vw\zlib-1.2.5)
    (d) Run "nmake /f win32\Makefile.msc"

(6) Get a copy of VW in %ROOT%. I ran "cd \src\vw" and "git clone http

    (a) Change to root (for me, "cd \src\vw")
    (b) "git clone https://github.com/chrisquirk/vowpal_wabbit.git"

(7) Open %ROOT%\vowpal_wabbit\vowpalwabbit\vw.sln in Visual Studio 2010 and hit Build.

(8) Build. Binaries will be in one of these two directories, based on whether you built DEBUG or RELEASE bits.

  %ROOT%\vowpal_wabbit\vowpalwabbit\Debug\vw.exe
  %ROOT%\vowpal_wabbit\vowpalwabbit\Release\vw.exe


