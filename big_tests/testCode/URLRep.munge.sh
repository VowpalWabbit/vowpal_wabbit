#! /bin/bash

tar xzf $1
cat url_svmlight/*.svm \
| $testCodeDir/svml2vw.pl
