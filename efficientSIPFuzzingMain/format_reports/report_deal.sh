#!/bin/bash
report=$(cat $1 | grep \| | cut -d \| -f 1)
#echo $report
r_max=0.1
for var in $report; do
	# echo $var
	set $var
	# echo $1
	# echo $(echo "$r_max < $(echo $1 | bc)" | bc)
	if [ $(echo "$r_max < $(echo $1 | bc)" | bc) -eq "1" ]; then
		r_max=$(echo $1 | bc)
		# echo $r_max
	fi		
done
echo $r_max

