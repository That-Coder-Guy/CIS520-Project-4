#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>


// Declares a structure type that represents a job that can be run by a worker thread in a thread pool.
// \attribute: data - A generic pointer to data to be processed.
// \attribute: size - The size of the data to be processed
// \attribute: result - The result of processing the provided data.
typedef struct thread_job
{
    void* data;
    size_t size;
    union {
        char as_char;
        int as_int;
        float as_float;
        void* as_pointer;
    } result;
} thread_job_t;


/// An opaque thread pool structure
typedef struct thread_pool thread_pool_t;

/// A function pointer type describing the expected thread worker function signature.
typedef void (*worker_func_t)(thread_job_t*);

/// A contructor for the thread_pool structure.
/// \param: thread_count - The number of threads to create for the pool.
/// \param: worker_delegate - A function that can process jobs.
/// \return: A generic pointer to a return value.
thread_pool_t* thread_pool_create(size_t thread_count, worker_func_t worker_delegate);

/// A destructor for the thread_pool structure.
/// \param: pool - The thread pool to destruct.
void thread_pool_destroy(thread_pool_t* pool);

/// Starts processing a provided job batch on the given thread pool.
/// \param: pool - The thread pool to process the job batch with.
/// \param: jobs - The job batch to process.
/// \param: job_count - The number of jobs in the provided batch.
void thread_pool_submit_batch(thread_pool_t* pool, thread_job_t* jobs, size_t job_count);

/// Waits until the given thread pool is finished processing its current batch.
/// \param: pool - The thread pool to wait on.
void thread_pool_wait(thread_pool_t* pool);

#endif