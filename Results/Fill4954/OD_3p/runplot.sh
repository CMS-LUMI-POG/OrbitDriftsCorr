#! /bin/bash

N_DRAW_STEER=1

POS_BPMs1=("DOROS"  "BPM1")
POS_BPMs2=("arcBPM" "BPM2")


POS_BPMid=-1

for (( id=0; id<${#POS_BPMs1[@]}; id++ ))
do    
    if [ -f data_steering_${POS_BPMs1[id]}.txt ]; then
        POS_BPMid=$id
        break
    fi
done

if [ $POS_BPMid == -1 ]; then
    echo "ERROR: Cannot find any of the next steerings:"
    for (( id=0; id<${#POS_BPMs1[@]}; id++ )); do
        echo "      data_steering_${POS_BPMs1[id]}.txt"
    done
    return 1 2> /dev/null || exit 1
fi

if [ ! -f data_steering_${POS_BPMs2[POS_BPMid]}.txt ]; then
    echo "ERROR: Found data_steering_${POS_BPMs1[POS_BPMid]}.txt but cannot find data_steering_${POS_BPMs2[POS_BPMid]}.txt ."
    return 1 2> /dev/null || exit 1
fi


STR_BPM1=$(awk '/Title:/ {print $2}' data_steering_${POS_BPMs1[POS_BPMid]}.txt)
STR_BPM2=$(awk '/Title:/ {print $2}' data_steering_${POS_BPMs2[POS_BPMid]}.txt)


STR_FILL=$(grep -io -m 1 "fill[0-9]\{4,\}" draw_steering_1.txt | head -1)


if [ "$STR_FILL" != "" ]; then
    STR_NAME_OD="${STR_FILL}_OrbitDrift_XY"
    STR_NAME_BPM1="${STR_FILL}_${POS_BPMs1[POS_BPMid]}"
    STR_NAME_BPM2="${STR_FILL}_${POS_BPMs2[POS_BPMid]}"
    STR_NAME_NOM="${STR_FILL}_Nominal"
else
    STR_NAME_OD="OrbitDrift_XY"
    STR_NAME_BPM1="${POS_BPMs1[POS_BPMid]}"
    STR_NAME_BPM2="${POS_BPMs2[POS_BPMid]}"
    STR_NAME_NOM="Nominal"
fi


for i in $(seq 1 $N_DRAW_STEER);
do
    
    ./plotVdMOrbitDrift_3p draw_steering_${i}.txt data_steering_${POS_BPMs1[POS_BPMid]}.txt data_steering_${POS_BPMs2[POS_BPMid]}.txt data_steering_Nominal.txt
    
    mv  VdM_OrbitDrift_XY.eps   ${STR_NAME_OD}_${i}.eps
    mv  VdM_${STR_BPM1}.eps     ${STR_NAME_BPM1}_${i}.eps
    mv  VdM_${STR_BPM2}.eps     ${STR_NAME_BPM2}_${i}.eps
    mv  VdM_Nominal.eps         ${STR_NAME_NOM}_$i.eps
    
done

