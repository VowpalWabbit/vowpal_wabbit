Help file for using this testing framework
------------------------------------------

If you ever forget what you're currently reading, you can always see it again by typing

make help

To run all the tests until one fails, type

make

To run all the tests regardless of failures, type

make -i

Either way, you would be making the default target, named 'valid'. (Naming it 'test' would get confusing when you start referring to data train/test splits.)

To clean up so that you can start over, type

make clean

To download and prepare all the relevant data at once, type

make prepData

To erase all the data, type

make eraseData

If you change some code so that the output of some tests changes, or you add some new tests, and you want to save the new output so that future outputs will be compared with it, type

make expected

You can work with individual tests by prepending their name to the make target, as in

make 2.valid
make t17.clean
make myNewTest.expected

Of course, you can make multiple targets together, as in

make 2.clean 2.valid

There are also pre-defined groups of tests that you can run, such as

make regression_group

If you want to create the correct outputs for a test without actually running it, type

make $testName.pretend

This can be useful if you want to run test X, which depends on the outputs of test Y, but you can't run test Y for some reason.

The "valid" target for each test is actually a "run" followed by a "compare".  Thus

make 3.run 3.compare

is equivalent to

make 3.valid

and it's possible to invoke either part without the other.

If you want to use an executable other than ../vowpalwabbit/vw, you can supply it on the command line like this:

make EXEC=myProgram

or make ../vowpalwabbit/vw a sym-link to it.

Other variables, such as ARCH, DIFF, TIME, WGET, and GREP, can also be set on the command line similarly.

If you want to pass extra parameters to all tests, set the EP variable, like this:

make EP="--random_seed 13"

If you want to check speed in addition to accuracy, invoke with TIMING=y .  Due to the many factors that can change execution speed, we do not expect speed results to be portable across machines.  Therefore, "expected" speed files are not checked into the repository.  If you'd like to do regression testing for speed, you should `make TIMING=y; make TIMING=y expected` on your particular machine before making any code changes.  In fact, it's a good idea to then `make TIMING=y` a couple more times, and `make TIMING=y expected` again if the speed drops significantly.  This can happen even on the same machine, if it's dividing attention between multiple processes.

If you want to add or modify tests, read HOWTO.write_new_tests.txt .


Working with data archives
--------------------------

To save time and bandwidth, you can create and retrieve all the data used in all the tests at once.  To create an archive of all the data, type

make ARF=<archive file> archive

You can then stash the archive wherever you like, probably at a URL-accessible location in the cloud.  Later, to retrieve the data again, you can type 

make URL=<url> prepData

This will cause the data to be downloaded and installed in prepared form, so that you don't need to run the preparation steps.
