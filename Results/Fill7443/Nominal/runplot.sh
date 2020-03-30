#! /bin/bash

N_DRAW_STEER=2

rm *.eps

for i in $(seq 1 $N_DRAW_STEER);
do
    
    echo $i
    
    ./plotVdMnom draw_steering_${i}.txt data_steering.txt

    mv VdMscan.eps          VdMscan_$i.eps

done
