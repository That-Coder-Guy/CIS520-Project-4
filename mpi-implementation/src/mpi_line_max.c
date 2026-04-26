#include "../include/mpi_line_max.h"

int compute_line_max(const char *line)
{
    int max = 0;

    for (int i = 0; line[i] != '\0' && line[i] != '\n'; i++)
    {
        unsigned char c = (unsigned char)line[i];
        if (c > max)
            max = c;
    }

    return max;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4)
    {
        if (rank == 0)
            printf("Usage: %s <file> <chunk_lines> <print_output>\n", argv[0]);

        MPI_Finalize();
        return 1;
    }

    char *filename = argv[1];
    int chunk_lines = atoi(argv[2]);
    int print_output = atoi(argv[3]);

    double start = MPI_Wtime();

    if (rank == 0)
    {
        FILE *fp = fopen(filename, "r");

        if (!fp)
        {
            printf("Could not open file.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        char line[MAX_LINE_LEN];
        int next_worker = 1;
        int global_line = 0;
        int active_workers = 0;

        while (1)
        {
            char *buffer = malloc(chunk_lines * MAX_LINE_LEN);
            int lines_read = 0;

            while (lines_read < chunk_lines &&
                   fgets(line, MAX_LINE_LEN, fp))
            {
                strcpy(buffer + (lines_read * MAX_LINE_LEN), line);
                lines_read++;
            }

            if (lines_read == 0)
            {
                free(buffer);
                break;
            }

            MPI_Send(&lines_read, 1, MPI_INT,
                     next_worker, TAG_WORK, MPI_COMM_WORLD);

            MPI_Send(buffer,
                     lines_read * MAX_LINE_LEN,
                     MPI_CHAR,
                     next_worker,
                     TAG_WORK,
                     MPI_COMM_WORLD);

            free(buffer);

            next_worker++;
            active_workers++;

            if (next_worker >= size)
                next_worker = 1;

            if (active_workers == size - 1)
            {
                int recv_count;
                MPI_Status status;

                MPI_Recv(&recv_count, 1, MPI_INT,
                         MPI_ANY_SOURCE, TAG_WORK,
                         MPI_COMM_WORLD, &status);

                int *results = malloc(sizeof(int) * recv_count);

                MPI_Recv(results, recv_count, MPI_INT,
                         status.MPI_SOURCE, TAG_WORK,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int i = 0; i < recv_count; i++)
                {
                    if (print_output)
                        printf("%d: %d\n", global_line++, results[i]);
                }

                free(results);
                active_workers--;
            }
        }

        while (active_workers > 0)
        {
            int recv_count;
            MPI_Status status;

            MPI_Recv(&recv_count, 1, MPI_INT,
                     MPI_ANY_SOURCE, TAG_WORK,
                     MPI_COMM_WORLD, &status);

            int *results = malloc(sizeof(int) * recv_count);

            MPI_Recv(results, recv_count, MPI_INT,
                     status.MPI_SOURCE, TAG_WORK,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int i = 0; i < recv_count; i++)
            {
                if (print_output)
                    printf("%d: %d\n", global_line++, results[i]);
            }

            free(results);
            active_workers--;
        }

        for (int i = 1; i < size; i++)
        {
            int zero = 0;
            MPI_Send(&zero, 1, MPI_INT, i, TAG_STOP, MPI_COMM_WORLD);
        }

        fclose(fp);

        double end = MPI_Wtime();
        printf("Elapsed Time: %.4f sec\n", end - start);
    }
    else
    {
        while (1)
        {
            int lines_read;
            MPI_Status status;

            MPI_Recv(&lines_read, 1, MPI_INT,
                     0, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_STOP)
                break;

            char *buffer = malloc(lines_read * MAX_LINE_LEN);

            MPI_Recv(buffer,
                     lines_read * MAX_LINE_LEN,
                     MPI_CHAR,
                     0,
                     TAG_WORK,
                     MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            int *results = malloc(sizeof(int) * lines_read);

            for (int i = 0; i < lines_read; i++)
            {
                results[i] = compute_line_max(
                    buffer + (i * MAX_LINE_LEN));
            }

            MPI_Send(&lines_read, 1, MPI_INT,
                     0, TAG_WORK, MPI_COMM_WORLD);

            MPI_Send(results, lines_read, MPI_INT,
                     0, TAG_WORK, MPI_COMM_WORLD);

            free(buffer);
            free(results);
        }
    }

    MPI_Finalize();
    return 0;
}