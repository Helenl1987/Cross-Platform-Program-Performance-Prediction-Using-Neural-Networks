#!/bin/bash
echo "Hello World!"
gem5=./gem5/build/ARM/gem5.opt
gem5config=./gem5/configs/example/se.py
obdir=./ContestData/ACMFinalsSolutions/finals2016
inrootdir=./ContestData/icpc2016data
flag=1

for obfile in $obdir/*
do
	# if [[ $flag == 2 ]];
	# 	then
	# 	break
	# fi
	if [[ ${obfile##*.} = "cc" ]];
		then
		continue
	fi
	echo $obfile
	aobfile=${obfile##*/}
	echo $aobfile
	# if [[ $aobfile = "G" ]];
	# 	then
	# 	con=1
	# else
	# 	continue
	# fi
	outdir="${obdir}/${aobfile}_gemout"
	if [[ ! -d "$outdir" ]]; 
		then
	    mkdir $outdir
	fi
	for indir in $inrootdir/*
	do
		# if [[ $flag == 2 ]];
		# 	then
		# 	break
		# fi
		tmpa=${indir%%-*}
		tmpa=${tmpa##*/}
		# echo $tmpa
		if [[ $tmpa = $aobfile ]];
			then
			aindir=${indir##*/}
			tickfile="${obdir}/${aindir}.tick"
			indir="${indir}/secret"
			echo "find match" $obfile $indir
			echo $tickfile
			for infile in $indir/*
			do
				# if [[ $flag == 2 ]];
				# 	then
				# 	break
				# fi
				if [[ ${infile##*.} = "in" ]];
					then
					ainfile=${infile##*/}
					ainfile=${ainfile%%.*}
					echo $ainfile
					resfile="${outdir}/${ainfile}.txt"
					echo $resfile
					echo "${gem5} ${gem5config} -c ${obfile} < ${infile} &> ${resfile}"
					gtimeout 120 ${gem5} ${gem5config} -c ${obfile} < ${infile} &> ${resfile}
					# if [[ ! -f "$resfile" ]]; 
					# 	then
					#     continue
					# fi
					output=$(grep "Exiting" $resfile)
					echo $output
					output=$(echo $output | grep -o "tick [0-9]*")
					echo $output
					output=(${output// / })
					output=${output[1]}
					echo $output >> $tickfile
					# flag=2
				fi
			done
		fi
	done
done