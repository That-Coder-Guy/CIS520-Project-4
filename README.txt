Instructions for pthread program:
- Set the current directory using the command 'cd pthread-implementation'
- Create a build directory using the command 'mkdir build'
- Set the current directory to the build folder using the command 'cd build'
- Load the required modules using the command 'module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0'
- Run the CMake command in the current directory using the command 'cmake ..'
- Run the Make command in the current directory using the command 'make'
- Set the current directory using the command 'cd ../testing'
- Run the resource tests in thethe current directory using the command './run_tests.sh'
- Once the tests have finished aggregate their results into a CSV file using the command './aggregate_results.sh'

Instructions for MPI program:
- Go into the mpi_implementation directory
- Load the required modules: module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0
- Run make to compile the program
- Run sbatch ./run_mpi.slurm to schedule the job

Instructions for OpenMP program:
- Go into the openmp_implementation directory
- Go to the build directory (create one if not already present)
- Run the cmake .. and make command in that order
- Go back to the openmp_implemenation directory
- Run sbatch ./opm_run.slurm