Instructions for MPI program:
Go into the mpi_implementation directory
load the required modules:
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0
run make to compile the program
run sbatch ./run_mpi.slurm to schedule the job

Instructions for OpenMP program:
Go into the openmp_implementation directory
go to the build director (create one if not already present)
run the cmake .. and make command in that order
go back to the openmp_implemenation directory
run sbatch ./opm_run.slurm