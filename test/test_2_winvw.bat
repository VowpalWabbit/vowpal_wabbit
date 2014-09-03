..\vowpalwabbit\x64\Debug\vw.exe -k -l 20 --initial_t 128000 --power_t 1 -d train-sets\0001.dat -f 0001A.model.tmp -c --passes 8 --invariant --ngram 3 --skips 1 
..\vowpalwabbit\x64\Debug\vw.exe -t -d test-sets\0001.dat -i 0001A.model.tmp --invariant --predictions 0001A.predict.tmp
..\vowpalwabbit\x64\Release\vw.exe -k -l 20 --initial_t 128000 --power_t 1 -d train-sets\0001.dat -f 0001B.model.tmp -c --passes 8 --invariant --ngram 3 --skips 1 
..\vowpalwabbit\x64\Release\vw.exe -t -d test-sets\0001.dat -i 0001B.model.tmp --invariant --predictions 0001B.predict.tmp
..\vowpalwabbit\x86\Debug\vw.exe -k -l 20 --initial_t 128000 --power_t 1 -d train-sets\0001.dat -f 0001C.model.tmp -c --passes 8 --invariant --ngram 3 --skips 1 
..\vowpalwabbit\x86\Debug\vw.exe -t -d test-sets\0001.dat -i 0001C.model.tmp --invariant --predictions 0001C.predict.tmp
..\vowpalwabbit\x86\Release\vw.exe -k -l 20 --initial_t 128000 --power_t 1 -d train-sets\0001.dat -f 0001D.model.tmp -c --passes 8 --invariant --ngram 3 --skips 1 
..\vowpalwabbit\x86\Release\vw.exe -t -d test-sets\0001.dat -i 0001D.model.tmp --invariant --predictions 0001D.predict.tmp
