#!/bin/bash
#SBATCH --job-name=hw4_omp
#SBATCH --nodes=1            #OpenMP threads must be on the same node
#SBATCH --cpus-per-task=20
#SBATCH --mem-per-cpu=1536M
#SBATCH --time=4:00:00
#SBATCH --constraint=moles
#SBATCH --output=omp_%j.csv  #.csv so that it can be opened in Excel or Sheets


#Sets up the environment
module purge
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0;

echo "Num of cores, Amount of Memory Per Core Time (seconds)"

for cores in 1 2 4 8 16 20 #tests can only go up to 20 because there are only 20 cores per node
do
	for mem in 64M 128M 256M 512M 1G 1536M #1536M instead of 1.5G so the program reads a whole number
	do                                     #leaving out 3GB because 3GB for 20 cores exceeds the memory of 1 node
		
	
		out=$(export OMP_NUM_THREADS=$cores; srun --ntasks=1 --cpus-per-task=$cores \
			--mem-per-cpu=$mem \
			/homes/zalatta/openmp_implementation/build/hw4_omp \
			/homes/eyv/cis520/wiki_dump.txt \
			5000 0 2>&1) #2>&1 redirects stderr to stdout

		time=$(echo "$out" | grep "Execution_Time" | cut -d':' -f2 | xargs)
		

		echo "$cores, $mem, $time"
	done
done
