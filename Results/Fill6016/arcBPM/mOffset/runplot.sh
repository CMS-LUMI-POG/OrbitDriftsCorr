#!/bin/bash

N_DRAW_STEER=5

POS_BPMs=("DOROS" "arcBPM" "BPM")


BPM_STEER=""

for ibpm in ${POS_BPMs[@]};
do
    if [ -f data_steering_${ibpm}.txt ]; then
        BPM_STEER=${ibpm}
        break
    fi
done

if [ $BPM_STEER == "" ]; then
    echo "ERROR: Cannot find any BPM steering !"
    exit 1
fi


STR_FILL=$(grep -io "fill[0-9]\{4,\}" draw_steering_1.txt)

if [ $STR_FILL != "" ]; then
    STR_NAME_BPM="${STR_FILL}_${BPM_STEER}"
    STR_NAME_NOM="${STR_FILL}_Nominal"
else
    STR_NAME_BPM="${BPM_STEER}"
    STR_NAME_NOM="Nominal"
fi


for i in $(seq 1 $N_DRAW_STEER);
do

    ./plotVdMbpm draw_steering_${i}.txt data_steering_${BPM_STEER}.txt data_steering_Nominal.txt

    mv VdMscan_BPM.eps ${STR_NAME_BPM}_${i}.eps
    mv VdMscan_BPM_offset.eps ${STR_NAME_BPM}_offset_${i}.eps
    mv VdMscan_Nom.eps ${STR_NAME_NOM}_${i}.eps
    
done

