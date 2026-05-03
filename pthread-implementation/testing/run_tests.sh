#!/bin/bash

CORES=(1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20)
MEMORY=("16M" "192M" "352M" "512M" "672M" "832M" "992M" "1152M" "1312M" "1472M" "1632M" "1792M" "1952M" "2112M" "2272M" "2432M" "2592M" "2752M" "2912M" "3072M")

mkdir -p test_results

for c in "${CORES[@]}"; do
    for m in "${MEMORY[@]}"; do
        
        JOB_NAME="test_${c}c_${m}"
        OUT_FILE="test_results/results_${c}cores_${m}.txt"
        
        echo "Submitting: $c Cores, $m Memory..."
        
        sbatch \
            --cpus-per-task=$c \
            --mem-per-cpu=$m \
            --job-name=$JOB_NAME \
            --output=$OUT_FILE \
            benchmark.slurm
            
    done
done

echo "All 400 benchmark jobs have been submitted to Beocat!"