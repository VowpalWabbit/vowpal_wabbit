#! /bin/bash

run_offline(){

  # parameters for all the experiments
  pass=1
  timee=5000
  pp=10000
  bb=18
  ll=1
  nn=4
  hh=1

  name=$1
  min=$2
  max=$3

  datatrain=$name\_train.dat
  datatest=$name\_test.dat
  data=$name.dat
  sdata=$name
  filename1=results/$sdata\_offline\_srm.txt
  filename2=results/$sdata\_offline\_test.txt

  # running the CATS offline algorithm and saving the loss estimation in SRM and test error results

  # 1) saving logged data from training the initial model

  time -p timeout $timee build/vowpalwabbit/cli/vw --cbify $nn --cbify_reg --min_value=$min --max_value=$max --bandwidth $hh \
  -d test/train-sets/regression/$datatrain --passes $pass -b $bb --coin --loss_option $ll -p results/$sdata\_$ll.acp

  python3 papers/${PAPER}/utility/acp_regression_data_join.py --p results/$sdata\_$ll.acp --d test/train-sets/regression/$datatrain > results/$sdata\_$ll.acpx

  # 2) offline training a new model and saving the loss estimation in SRM and test error results

  printf "." > $filename1
  printf "." > $filename2

  for n in 4 8 16 32 64 128 256 512 1024 2048 4096 8192 #16384
  do
  for h in 1 2 4 8 16 32 64 128 256 512 1024 2048 #4096
  do
  if (( $n > 2*$h ));
  then
  printf "\nCATS-offline" >> $filename1
  printf "\nn = $n" >> $filename1
  printf "\nh = $h" >> $filename1
  printf "\ntime -p timeout $timee build/vowpalwabbit/cli/vw --cats $n --min_value=$min --max_value=$max --bandwidth $h \
  -d results/$sdata\_$ll.acpx --passes $pass -b $bb --coin --loss_option $ll -f results/$sdata.m -p results/$sdata\_$ll\_$n\_$h.ap \n" >> $filename1
  time -p timeout $timee build/vowpalwabbit/cli/vw --cats $n --min_value=$min --max_value=$max --bandwidth $h \
  -d results/$sdata\_$ll.acpx --passes $pass -b $bb --coin --loss_option $ll -f results/$sdata.m -p results/$sdata\_$ll\_$n\_$h.ap
  python3 papers/${PAPER}/utility/srm.py --p results/$sdata\_$ll\_$n\_$h.ap --d results/$sdata\_$ll.acp -m $max -i $min -k $n --bandwidth $h >> $filename1
  printf "\nCATS-offline" >> $filename2
  printf "\nn = $n" >> $filename2
  printf "\nh = $h" >> $filename2
  printf "\ntime -p timeout $timee build/vowpalwabbit/cli/vw --cats $n --min_value=$min --max_value=$max --bandwidth $h \
  -d test/train-sets/regression/$datatest --passes $pass -b $bb --coin --loss_option $ll -i results/$sdata.m -p results/$sdata.ap -t \n" >> $filename2
  time -p timeout $timee build/vowpalwabbit/cli/vw --cats $n --min_value=$min --max_value=$max --bandwidth $h \
  -d test/train-sets/regression/$datatest --passes $pass -b $bb --coin --loss_option $ll -i results/$sdata.m -p results/$sdata.ap -t
  python3 papers/${PAPER}/utility/ap_regression_data_join.py --p results/$sdata.ap --d test/train-sets/regression/$datatest -m $max -i $min >> $filename2

  fi;
  done
  done
}


run_online(){

  # parameters for all the experiments
  pass=1
  timee=5000
  pp=10000
  bb=18
  ll=1
  nn=4
  hh=1

  name=$1
  min=$2
  max=$3

  datatrain=$name\_train.dat
  datatest=$name\_test.dat
  data=$name.dat
  sdata=$name
  filename3=results/$sdata\_online_validation.txt

  # running the online algorithms for CATS as well as the comparators and saving the progressive validation results

  echo "." > $filename3

  for n in 4 8 16 32 64 128 256 512 1024 2048 4096 8192 #16384
  do
  for h in 1 2 4 8 16 32 64 128 256 512 1024 2048 #4096
  do
  if (( $n > 2*$h ));
  then
  printf "\nCATS-online" >> $filename3
  printf "\nn = $n" >> $filename3
  printf "\nh = $h" >> $filename3
  printf "\ntime -p timeout $timee build/vowpalwabbit/cli/vw --cbify $n --cbify_reg --min_value=$min --max_value=$max --bandwidth $h \
  -d test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll \n\n" >> $filename3
  time -p ( timeout $timee build/vowpalwabbit/cli/vw --cbify $n --cbify_reg --min_value=$min --max_value=$max --bandwidth $h \
  -d test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll >> $filename3 2>&1 ) 2>> $filename3
  fi;
  done
  done


  printf "\n\n-----------------------------------------------------------------------------" >> $filename3
  for n in 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192
  do
  printf "\n\nDiscretized-Tree-online" >> $filename3
  printf "\nn = $n" >> $filename3
  printf "\ntime -p timeout $timee build/vowpalwabbit/cli/vw --cbify $n --cbify_reg --cb_discrete --cats_tree $n --min_value=$min --max_value=$max \
  -d test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll \n\n" >> $filename3
  time -p ( timeout $timee build/vowpalwabbit/cli/vw --cbify $n --cbify_reg --cb_discrete --cats_tree $n --min_value=$min --max_value=$max \
  -d test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll >> $filename3 2>&1 ) 2>> $filename3
  done


  printf "\n\n----------------------------------------------------------------------------" >> $filename3
  for n in 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192
  do
  printf "\n\nDiscretized-Linear-online" >> $filename3
  printf "\nn = $n" >> $filename3
  printf "\ntime -p timeout $timee build/vowpalwabbit/cli/vw --cbify $n --cbify_reg --cb_discrete --min_value=$min --max_value=$max \
  -d test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll \n\n" >> $filename3
  time -p ( timeout $timee build/vowpalwabbit/cli/vw --cbify $n --cbify_reg --cb_discrete --min_value=$min --max_value=$max \
  -d test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll >> $filename3 2>&1 ) 2>> $filename3
  done
}