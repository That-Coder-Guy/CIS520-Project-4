#ifndef MPI_LINE_MAX_H
#define MPI_LINE_MAX_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

#define MAX_LINE_LEN 8192
#define TAG_WORK 1
#define TAG_STOP 2

int compute_line_max(const char *line);

#endif