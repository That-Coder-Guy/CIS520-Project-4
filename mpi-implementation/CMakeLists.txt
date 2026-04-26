cmake_minimum_required(VERSION 3.10)
project(hw4_mpi C)

find_package(MPI REQUIRED)

include_directories(
    include
    ${MPI_C_INCLUDE_PATH}
)

add_executable(hw4_mpi
    src/mpi_line_max.c
)

target_link_libraries(hw4_mpi ${MPI_C_LIBRARIES})