#!/bin/bash
#SBATCH --time=00:15:00
#SBATCH --constraint=moles
#SBATCH --nodes=1
#SBATCH --ntasks=1

echo "---------------------------------------------------"
echo "TEST CONFIG: $SLURM_CPUS_PER_TASK Cores | $SLURM_MEM_PER_CPU Mem per Core"
echo "---------------------------------------------------"

/usr/bin/time -v ../build/pthread_implementation > /dev/null