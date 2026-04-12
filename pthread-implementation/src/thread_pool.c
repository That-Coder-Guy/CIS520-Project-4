#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "thread_pool.h"

// Declares a structure that synchronizes a collction of threads to process job batches.
// \attribute: lock - A lock for job related resources
// \attribute: notify - A condition for waking threads to start a processing a job batch.
// \attribute: batch_done - A condition for waking threads when batch processing has finished.
// \attribute: shutdown - A boolean used to determine the state of worker threads in the pool.
// \attribute: worker_delegate - A function capable of processing a job.
// \attribute: thread_count - The number of worker threads in the pool.
// \attribute: threads - The thread handles of the worker threads.
// \attribute: jobs - The current batch of jobs.
// \attribute: job_count - The number of jobs in the current batch.
// \attribute: next_job - The index of the next job that has yet to be processed.
// \attribute: jobs_finished - A boolean represening whether the current batch has been entirely processed.
struct thread_pool
{
    // Syncronization attributes
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_cond_t batch_done;
    bool shutdown;

    // Worker thread attributes
    worker_func_t worker_delegate;
    size_t thread_count;
    pthread_t* threads;

    // Job tracking attributes
    thread_job_t* jobs;
    size_t job_count;
    size_t next_job;
    size_t jobs_finished;
};

/// A pthread compatible function that handles the aquasition and completion tracking of thrad pool jobs.
/// \param: arg - A generic pointer to an input value.
/// \return: A generic pointer to a return value.
void* thread_worker(void* arg)
{
    // Cast the generic argument value to the expected thread pool pointer type
    thread_pool_t* pool = (thread_pool_t*)arg;
    if (pool == NULL) { return NULL; }

    // Loop until the thread pool signals a shutdown
    while (true)
    {
        // Acquire the job batch lock
        pthread_mutex_lock(&pool->lock);

        // If no jobs are in the batch or the batch is finished then sleep util woken
        while ((pool->jobs == NULL || pool->jobs_finished == pool->job_count) && !pool->shutdown)
        {
            pthread_cond_wait(&pool->notify, &pool->lock);
        }

        // Terminate if the thread pool signaled a shutting down
        if (pool->shutdown && (pool->jobs == NULL || pool->jobs_finished == pool->job_count))
        {
            pthread_mutex_unlock(&pool->lock);
            break; 
        }

        // Claim the next job in the batch
        thread_job_t* job = NULL;
        if (pool->next_job < pool->job_count) 
        {
            job = &pool->jobs[pool->next_job];
            pool->next_job++;
        }
        
        // Release the job batch lock before executing job
        pthread_mutex_unlock(&pool->lock);

        // Execute job
        if (job != NULL)
        {
            pool->worker_delegate(job);
            
            // Re-acquire the job batch lock and mark the current job as finished
            pthread_mutex_lock(&pool->lock);
            pool->jobs_finished++;
            
            // If the current job was the last one to finish then wake all threads waiting on results
            if (pool->jobs_finished == pool->job_count)
            {
                pthread_cond_signal(&pool->batch_done);
            }

            // Release the job batch lock
            pthread_mutex_unlock(&pool->lock);
        }
    }
    return NULL;
}

/// A contructor for the thread_pool structure.
/// \param: thread_count - The number of threads to create for the pool.
/// \param: worker_delegate - A function that can process jobs.
/// \return: A generic pointer to a return value.
thread_pool_t* thread_pool_create(size_t thread_count, worker_func_t worker_delegate)
{
    // Create the initial thread pool structure
    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    if (pool == NULL) { return NULL; }

    // Initialize syncronization attributes
    if (pthread_mutex_init(&pool->lock, NULL) != 0)
    {
        free(pool);
        return NULL;
    }
    if (pthread_cond_init(&pool->notify, NULL) != 0)
    {
        pthread_mutex_destroy(&pool->lock);
        free(pool);
        return NULL;   
    }
    if (pthread_cond_init(&pool->batch_done, NULL) != 0)
    {
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->notify);
        free(pool);
        return NULL;   
    }
    pool->shutdown = false;

    // Initialize worker thread attributes
    pool->worker_delegate = worker_delegate;
    pool->thread_count = thread_count;
    pool->threads = malloc(sizeof(pthread_t) * pool->thread_count);
    if (pool->threads == NULL)
    {
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->notify);
        pthread_cond_destroy(&pool->batch_done);
        free(pool);
        return NULL;
    }

    for (size_t i = 0; i < pool->thread_count; i++)
    {
        if (pthread_create(&(pool->threads[i]), NULL, thread_worker, (void*)pool) != 0)
        {
            // Wake up and terminate any threads that were successfully created
            pthread_mutex_lock(&pool->lock);
            pool->shutdown = true;
            pthread_cond_broadcast(&pool->notify);
            pthread_mutex_unlock(&pool->lock);

            for (size_t j = 0; j < i; j++)
            {
                pthread_join(pool->threads[j], NULL);
            }
            
            pthread_mutex_destroy(&pool->lock);
            pthread_cond_destroy(&pool->notify);
            pthread_cond_destroy(&pool->batch_done);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }

    // Initialize job tracking attributes
    pool->jobs = NULL; 
    pool->job_count = 0;
    pool->next_job = 0;
    pool->jobs_finished = 0;

    // Return the newly created thread pool structure
    return pool;
}

/// A destructor for the thread_pool structure.
/// \param: pool - The thread pool to destruct.
void thread_pool_destroy(thread_pool_t* pool)
{
    // Validate input value
    if (pool == NULL) { return; }

    // Acquire the job batch lock
    pthread_mutex_lock(&pool->lock);

    // Wake up all worker threads and signal a shutdown
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->notify);

    // Release the job batch lock
    pthread_mutex_unlock(&pool->lock);

    // Wait for each worker thread to terminate or forcefully kill them if they resist
    for (size_t i = 0; i < pool->thread_count; i++)
    {
        if (pthread_join(pool->threads[i], NULL) != 0)
        {
            pthread_cancel(pool->threads[i]);
        }
    }

    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify); 
    pthread_cond_destroy(&pool->batch_done);

    free(pool->threads);

    // Free the thread pool structure
    free(pool);
}

/// Starts processing a provided job batch on the given thread pool.
/// \param: pool - The thread pool to process the job batch with.
/// \param: jobs - The job batch to process.
/// \param: job_count - The number of jobs in the provided batch.
void thread_pool_submit_batch(thread_pool_t* pool, thread_job_t* jobs, size_t job_count)
{
    // Validate input values
    if (pool == NULL || jobs == NULL || job_count == 0) return;
    
    // Acquire the job batch lock
    pthread_mutex_lock(&pool->lock);

    // Update all the thread pool's job batch attributes
    pool->jobs = jobs;
    pool->job_count = job_count;
    pool->next_job = 0;
    pool->jobs_finished = 0;
    
    // Wake up all workers
    pthread_cond_broadcast(&pool->notify);

    // Release the job batch lock
    pthread_mutex_unlock(&pool->lock);
}

/// Waits until the given thread pool is finished processing its current batch.
/// \param: pool - The thread pool to wait on.
void thread_pool_wait(thread_pool_t* pool)
{
    // Validate input value
    if (pool == NULL) return;

    // Acquire the job batch lock
    pthread_mutex_lock(&pool->lock);
    
    // Block until the workers signal that batch is finished
    while (pool->jobs_finished < pool->job_count)
    {
        pthread_cond_wait(&pool->batch_done, &pool->lock);
    }
    
    // Remove the thread pool's refernce to the current batch
    pool->jobs = NULL; 
    pool->job_count = 0;

    // Release the job batch lock
    pthread_mutex_unlock(&pool->lock);
}