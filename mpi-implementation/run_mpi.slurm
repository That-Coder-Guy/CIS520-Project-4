#!/bin/bash
#SBATCH --job-name=hw4_mpi
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=4
#SBATCH --mem-per-cpu=1G
#SBATCH --time=01:00:00
#SBATCH --constraint=mole
#SBATCH --output=mpi_%j.out

module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0

cd $SLURM_SUBMIT_DIR

make

srun ./build/hw4_mpi ~eyv/cis520/wiki_dump.txt 4000 0