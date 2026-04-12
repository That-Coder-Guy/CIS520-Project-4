#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include "thread_pool.h"


// Declares a batch structure that stores a list of thread jobs and associated information
// \attribute: jobs - An array of jobs.
// \attribute: job_count - The number of jobs in the array.
// \attribute: size - The total number of bytes contained with the data of all the jobs.
typedef struct job_batch
{
    thread_job_t* jobs;
    size_t job_count;
    size_t size;
} job_batch_t;

/// Attempts to read a specified nubmer of characters from a file.
/// \param: file_descriptor - A read-only file descriptor.
/// \param: buffer - The character buffer to read characters to.
/// \param: buffer_size - The size of the provided character buffer.
/// \return: The number of characters actually read, or -1 on error.
ssize_t read_characters(int file_descriptor, char* buffer, size_t buffer_size)
{
	// Valiate input values
	if (file_descriptor <= 0 || buffer == NULL || buffer_size == 0) return -1;

    // Loop and read from the input file into the buffer
    size_t total_characters_read = 0;
    while (total_characters_read < buffer_size)
    {
        // Make a read request to the kernel
        ssize_t characters_read = read(file_descriptor, buffer + total_characters_read, buffer_size - total_characters_read);
        
        // If characters were read increment the total
        if (characters_read > 0) { total_characters_read += characters_read; }
        // If the eof of the file is reached, return the total characters read
        else if (characters_read == 0) { return total_characters_read; }
        // If a system interrupt occured skip to the next iteration
        else if (characters_read == -1 && errno == EINTR) { continue; }
        // Else an error has occured therefore an error value should be returned
        else { return -1; }
    }

    // Return the actual number of characters read from the input file
	return total_characters_read;
}

/// Attempts to parse a character buffer into a batch of jobs each containing a single line.
/// \param: file_descriptor - A read-only file descriptor.
/// \param: buffer - The character buffer to parse.
/// \param: buffer_size - The size of the provided character buffer.
/// \param: is_eof - Whether the provided buffer contains the final characters in a file.
/// \return: A batch of jobs.
job_batch_t parse_buffer_to_jobs(char* buffer, size_t buffer_size, bool is_eof)
{
    // Initialize output struct
    job_batch_t result = { NULL, 0, 0 };

    // Validate input values
    if (buffer == NULL || buffer_size == 0) return result;

    // Count the number of complete lines in the buffer
    for (size_t i = 0; i < buffer_size; i++)
    {
        if (buffer[i] == '\n') { result.job_count++; }
    }

    // If the buffer contains the end of the input file but does not end cleanly with a
    // newline, then we have one final line to process.
    if (is_eof && buffer[buffer_size - 1] != '\n') { result.job_count++; }

    // If no lines where found in the buffer and the buffer does not contain the end
    // of the input file then return an empty batch.
    if (result.job_count == 0) return result;

    // Allocate a new array of jobs with each job corresponding to a complete line
    result.jobs = malloc(sizeof(thread_job_t) * result.job_count);
    if (result.jobs == NULL) 
    {
        result.job_count = 0;
        return result;
    }

    // Put each complete line in a job
    char* current_line_start = buffer;
    size_t current_job_index = 0;
    
    for (size_t i = 0; i < buffer_size; i++)
    {
        if (buffer[i] == '\n')
        {
            // Calculate the length of the complete line
            size_t line_length = (&buffer[i] - current_line_start);
            
            // Populate its corresponding job struct
            result.jobs[current_job_index].data = current_line_start;
            result.jobs[current_job_index].size = line_length;
            result.jobs[current_job_index].result.as_pointer = NULL;
            result.size = (i + 1);
            
            // Prepare to process another line
            current_job_index++;
            current_line_start = &buffer[i + 1];
        }
    }

    // Put remaining overflow bytes into one final job struct
    if (is_eof && result.size < buffer_size)
    {
        // Calculate the length of the complete line
        size_t line_length = buffer_size - result.size;
        
        // Populate its corresponding job struct
        result.jobs[current_job_index].data = current_line_start;
        result.jobs[current_job_index].size = line_length;
        result.jobs[current_job_index].result.as_pointer = NULL;
        result.size = buffer_size; 
    }

    // Returned the batch of parsed lines
    return result;
}

/// Finds the largest ASCII character within the data of the provided job amd sets
/// it as trhat job's result.
/// \param: job - A job to be processed.
void find_largest_ascii(thread_job_t* job)
{
    // Validate input value
    if (job == NULL || job->data == NULL || job->size == 0)  { return; }

    // Extract the file line fro mthe job struct and calculate its largest character value
    unsigned char* file_line = (unsigned char*)job->data;
    unsigned char largest_character = file_line[0];

    for (size_t i = 1; i < job->size && largest_character != UCHAR_MAX; i++)
    {
        if (largest_character < file_line[i]) { largest_character = file_line[i]; }
    }

    // Save the largest character value in the job struct
    job->result.as_char = (char)largest_character;
}

/// Program entry point function.
int main()
{

    const size_t CHARACTERS_PER_READ = 1000000; 

    // Create a double buffer for reading from an input file
    char* buffers[2];
    buffers[0] = malloc(sizeof(char) * CHARACTERS_PER_READ);
    buffers[1] = malloc(sizeof(char) * CHARACTERS_PER_READ);
    if (buffers[0] == NULL || buffers[1] == NULL)
    {
        fprintf(stderr, "Error: Not enough memeory to allocate double buffer.\n");

        if (buffers[0] != NULL) { free(buffers[0]); }
        if (buffers[1] != NULL) { free(buffers[1]); }
        return -1;
    }

    // Open the wiki dump text file for reading
    int file_descriptor = open("/homes/eyv/cis520/wiki_dump.txt", O_RDONLY);
    if (file_descriptor == -1)
    {
        fprintf(stderr, "Error: Failed to open input file (code %d).\n", errno);

        free(buffers[0]);
        free(buffers[1]);
        return -1;
    }

    // Create a thread pool for job batch processing
    const size_t THREAD_COUNT = sysconf(_SC_NPROCESSORS_ONLN);
    thread_pool_t* pool = thread_pool_create(THREAD_COUNT, find_largest_ascii);
    if (pool == NULL)
    {
        fprintf(stderr, "Error: Failed to create thread pool (code %d).\n", errno);

        free(buffers[0]);
        free(buffers[1]);
        close(file_descriptor);
        return -1;
    }

    // Initialize variable to track the state of the double buffers and job batches
    int active_buffer_index = 0;
    int read_buffer_index = 1;
    size_t buffer_sizes[2] = {0};
    job_batch_t batches[2] = {0};

    // Read the first chunk from the input file
    ssize_t characters_read = read_characters(file_descriptor, buffers[active_buffer_index], CHARACTERS_PER_READ);
    if (characters_read <= 0)
    {
        fprintf(stderr, "Error: Failed to read from input file (code %d).\n", errno);

        free(buffers[0]);
        free(buffers[1]);
        close(file_descriptor);
        thread_pool_destroy(pool);
        return -1;
    }
    buffer_sizes[active_buffer_index] = characters_read;
    bool is_eof = ((size_t)characters_read < CHARACTERS_PER_READ);
    
    // Split the first chunk into lines and process each with the thread pool asynchronously
    batches[active_buffer_index] = parse_buffer_to_jobs(buffers[active_buffer_index], buffer_sizes[active_buffer_index], is_eof);
    thread_pool_submit_batch(pool, batches[active_buffer_index].jobs, batches[active_buffer_index].job_count);

    size_t current_line_number = 0;
    while (true)
    {
        // If the current chunk isn't the last then read the next chunk while the thread pool is busy
        if (!is_eof)
        {
            // Calcualte the number of remaining characters after line parsing
            size_t overflow = buffer_sizes[active_buffer_index] - batches[active_buffer_index].size;

            // Exit with an error if no complete lines were found in the active buffer
            if (overflow == CHARACTERS_PER_READ)
            {
                fprintf(stderr, "Error: A line in the input file is longer than the read buffer (code %d).\n", errno);

                free(buffers[0]);
                free(buffers[1]);
                close(file_descriptor);
                thread_pool_destroy(pool);
                return -1;
            }

            // Move the overflow from the active buffer to the read buffer
            if (overflow > 0)
            {
                memcpy(buffers[read_buffer_index], 
                       buffers[active_buffer_index] + batches[active_buffer_index].size, 
                       overflow);
            }

            // Read from the input file into ther read buffer accounting for overflow
            size_t space_remaining = CHARACTERS_PER_READ - overflow;
            characters_read = read_characters(file_descriptor, buffers[read_buffer_index] + overflow, space_remaining);
            if (characters_read == 0) { is_eof = true; }
            else if (characters_read == -1)
            {
                fprintf(stderr, "Error: Failed to read from input file (code %d).\n", errno);

                free(buffers[0]);
                free(buffers[1]);
                close(file_descriptor);
                thread_pool_destroy(pool);
                return -1;
            }
            buffer_sizes[read_buffer_index] = overflow + (characters_read > 0 ? characters_read : 0);
            
            // Parse the new buffer into jobs
            batches[read_buffer_index] = parse_buffer_to_jobs(buffers[read_buffer_index], buffer_sizes[read_buffer_index], is_eof);
        }

        // Wait for the pool to finish processing the the active buffer before swapping it with the read buffer
        thread_pool_wait(pool);

        // Print the results of the finished jobs
        thread_job_t* completed_jobs = batches[active_buffer_index].jobs;
        for (size_t i = 0; i < batches[active_buffer_index].job_count; i++)
        {
            printf("%lu: %d\n", current_line_number++, (unsigned char)completed_jobs[i].result.as_char);
        }

        // Free the memory used by finished job batch
        free(completed_jobs);
        batches[active_buffer_index].job_count = 0;

        // If EOF was hit and there are no new jobs then exit the loop
        if (is_eof && batches[read_buffer_index].job_count == 0) { break; }

        // Swap the active and read buffers
        int temp = active_buffer_index;
        active_buffer_index = read_buffer_index;
        read_buffer_index = temp;

        // Submit the newly created job batch to the thread pool
        thread_pool_submit_batch(pool, batches[active_buffer_index].jobs, batches[active_buffer_index].job_count);
    }

    // Free all heap memory used
    free(buffers[0]);
    free(buffers[1]);
    close(file_descriptor);
    thread_pool_destroy(pool);
    return 0;
}