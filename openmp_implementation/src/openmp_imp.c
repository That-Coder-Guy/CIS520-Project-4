#include "../include/openmp_imp.h"

#define MAX_LINE_LENGTH 100000 //large max line length

int compute_max(const char*line){
	int max = 0;
	for(int i = 0; line[i] != '\0' && line[i] != '\n'; i++){
		int decimal = (int)line[i];
		if(decimal > max){
			max = decimal;
		}
	}

	return max;
}

///Runs the program
//\argc: the number of arguments
//\argv: the arguments themselves
//returns if the program was successful(1) or not (0)
int main(int argc, char *argv[]){

	if (argc != 4){
		fprintf(stderr, "Format: %s <text_file> <chunk_lines> <print_out>\n", argv[0]);
		return 0;
	}
	
	
	//name of file
	char *fn = argv[1];

	//amount of lines that will be passed to threads
	int line_chunk = atoi(argv[2]);

	//whether to print the output
	int print_output = atoi(argv[3]);
	
	//keeps track of total lines
	int lines_read = 0;
	

	FILE *fp = fopen(fn, "r");

	if(fp == NULL){
		fprintf(stderr, "File %s not found", fn);
		return 0;
	}

	//gets the start time of the program
        double start = omp_get_wtime();
	
	//contains all of the lines in the chunk
	char (*lines)[MAX_LINE_LENGTH] = malloc(line_chunk * sizeof(*lines));
	int *result = malloc(line_chunk * sizeof(int));
	
	//keeps track of the number of lines being read in each iteration
	int count = 0;	
	//gets threads ready
	#pragma omp parallel
	while(1){
		
		//fills the 'lines' array in segments of line_chunks
		//only one thread resets count and fills the chunck
		#pragma omp single
		{
		count = 0;
		while(count < line_chunk && fgets(lines[count], MAX_LINE_LENGTH, fp)){
			count++;
			lines_read ++;
		}
		}
		
		//if no lines were read, that means that the end of the file was reached
		if(count == 0){
			break;
		}

		//dynamic so threads are scheduled in fcfs format
		#pragma omp for schedule(dynamic)
		
			for(int i = 0; i < line_chunk; i++){
				result[i] = compute_max(lines[i]);
			}
		
		//prints output if user requests it
		#pragma omp single
		if(print_output == 1){
			for(int i = 0; i < line_chunk; i++){
				fprintf(stderr, "line %d value: %d\n",(lines_read -(line_chunk - i)), result[i]);
			}
		}
	}
	
	//gets programs end time
	double end = omp_get_wtime();
	
	fprintf(stderr, "-------REPORT------\nLines per Chunk %d\nLines Read: %d\n", line_chunk, lines_read);
	
	//separated so that it the time can be grabbed by the slurm code
	printf("Execution_Time: %.4f sec\n", end-start);
	free(lines);
	free(result);
	return 1;
}
