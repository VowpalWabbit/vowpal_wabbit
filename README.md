```
/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
```

[![Build Status](https://travis-ci.org/JohnLangford/vowpal_wabbit.png)](https://travis-ci.org/JohnLangford/vowpal_wabbit)

This is the vowpal wabbit fast online learning code.  For Windows, look at README.windows.txt

You can download the latest version from here:
https://github.com/JohnLangford/vowpal_wabbit/wiki/Download

Alternatively, the very latest version is available here:

```
git clone git://github.com/JohnLangford/vowpal_wabbit.git
```

You should be able to build it on most systems with:
make
(make test)

If that fails, try:
```
./autogen.sh
./configure
make
(make test)
make install
```

Note that ``./autogen.sh`` requires automake.  On OSX, this implies installing 'glibtools'

Be sure to read the wiki: https://github.com/JohnLangford/vowpal_wabbit/wiki
for the tutorial, command line options, etc.  

The 'cluster' directory has it's own documentation for cluster
parallel use, and the examples at the end of test/Runtests give some
example flags.
