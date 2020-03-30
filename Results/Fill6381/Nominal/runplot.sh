#! /bin/bash

rm *.eps
#rm plotVdMbpm
#make

for i in $(seq 1 1);
do
    
    echo $i
    
    ./plotVdMnom draw_steering_${i}.txt data_steering.txt

    mv VdMscan.eps          VdMscan_$i.eps

done
