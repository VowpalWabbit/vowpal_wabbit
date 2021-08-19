#! /bin/bash

# Author: Lekan Molu | August 14, 2021

filenm="../notes/result_$(date +%F_%T).dat" 
# filenm="../notes/result_Aug_14.dat" 

echo -e "Started Training CB and IGL on $(date)." | tee  "$filenm"
echo -e "Saving to $filenm" >> "$filenm" | tee -a "$filenm"
echo -e "|: $( seq -s- 102|tr -d '[:digit:]' ):|" | tee -a "$filenm"
echo -e "|Filename\t|\tNum Examples\t|\tCB Loss|IGL Loss\t|\tIGL Flipped Sign Loss|" | tee -a "$filenm"
echo -e "|: $( seq -s- 102|tr -d '[:digit:]' ):|" | tee -a "$filenm"

for f in data/*.gz;
	do
	   fn=(${f//// });  # separate filename from directory
	   fnm=${fn[-1]}    # get filename
	   #echo $fnm        # show me what you got

	   # get class name
	   cn=(${fnm//_/ })
	   last=${cn[-1]}; # get num classes
	   lastsplit=(${last//./ })
	   numclass=${lastsplit[0]}

	   #echo -e ">>>>>Going on $fnm: |\t with $numclass classes.>>>>>>\n"

	   # now call cbify
	   exec_cb="vowpalwabbit/vw $f --cbify $numclass --loss0 -1 --loss1 1"
	   exec_igl="vowpalwabbit/vw  --igl --cbify $numclass $f --loss0 -1 --loss1 1" # 2>&1 | grep "average loss""
		exec_igl_flip="vowpalwabbit/vw   --igl --cbify $numclass $f --loss0 -1 --loss1 1 --flip_loss_sign"
	   ( echo -e "$fnm \t" ;   $exec_cb  2>&1 |  grep 'average loss \|number of examples' | awk 'FNR == 1 { print $5 }; FNR == 2 {print $4 }' | tr '\n' '\t'; \
				$exec_igl  2>&1 | grep "average loss" | awk '{print $4}' ; \
				$exec_igl_flip  2>&1 | grep "average loss" | awk '{print $4}' ; \
				) |tr '\n' '\t'
        printf '\n'
	done  | tee -a "$filenm"
    
echo  -e "Finished Training CB and IGL on $(date +%F_%T)." | tee -a "$filenm"