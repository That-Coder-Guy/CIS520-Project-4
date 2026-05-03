#ifndef OPENMP_IMP_H
#define OPENMP_IMP_H

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

///Takes a line and iterates through each character, comparing each character's decimal value and then
// returning the highest value
// \param: line - the string of characters that will be iterated throough
// \return: The largest  decimal value
int compute_max (const char*line);

#endif
