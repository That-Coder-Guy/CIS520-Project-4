cmake_minimum_required(VERSION 3.10)

project(hw4_omp)

find_package(OpenMP REQUIRED)

add_executable(hw4_omp "$ENV{HOME}/openmp_implementation/src/openmp_imp.c")

target_include_directories(hw4_omp PRIVATE include "$ENV{HOME}/openmp_implementation/include")

if(OpenMP_C_FOUND)
        target_link_libraries(hw4_omp PRIVATE OpenMP::OpenMP_C)
endif()



