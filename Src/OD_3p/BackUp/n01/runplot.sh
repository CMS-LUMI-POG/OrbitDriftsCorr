#! /bin/bash

rm *.eps
#rm plotVdMbpm
#make

for i in $(seq 1 3);
do
    
    echo $i
    
    ./plotVdMorbitDrift_3p draw_steering_${i}.txt data_steering_BPM1.txt data_steering_BPM2.txt data_steering_Nominal.txt
    
    mv  VdM_OrbitDrift_XY.eps   VdM_Fill6868_OrbitDrift_XY_$i.eps
    mv  VdM_DOROS.eps           VdM_Fill6868_DOROS_$i.eps
    mv  VdM_arcBPM.eps          VdM_Fill6868_arcBPM_$i.eps
    mv VdM_Nominal.eps          VdM_Fill6868_Nominal_$i.eps

done

