cmake_minimum_required (VERSION 2.8)
project(hw4)

# Apply your exact C flags, adding -pthread which is required for the <pthread.h> library
set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror -D_XOPEN_SOURCE=500 -pthread")

# Global include directory
include_directories("${PROJECT_SOURCE_DIR}/include")

# If you are running the Wikipedia file parser yourself via a main.c
add_executable(thread_pool src/main.c src/thread_pool.c)