// Matrix multiplication

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUMTHRDS 4
double **a, **b, **c;
int matrixSize, nmats;
pthread_mutex_t mutex;

double **allocateMatrix() {
	int i;
	double *vals, **temp;

	// allocate space for values of a matrix
	vals = (double *) malloc (matrixSize * matrixSize * sizeof(double));

	// allocate vector of pointers to create the 2D array
	temp = (double **) malloc (matrixSize * sizeof(double*));

	for(i=0; i < matrixSize; i++)
		temp[i] = &(vals[i * matrixSize]);

	return temp;
}

void mm(int start, int end) {
	int i,j,k;
	double sum;
	// matrix multiplication
    for (i = start; i < end; i++) {
        for (j = 0; j < matrixSize; j++) {
            sum = 0.0;
            // dot product
            for (k = 0; k < matrixSize; k++) {
                sum = sum + a[i][k] * b[k][j];
            }
            // Use mutex to avoid race conditions when writing to the result matrix
            pthread_mutex_lock(&mutex);
            c[i][j] = sum;
            pthread_mutex_unlock(&mutex);
        }
    }
}

void *threadMultiply(void *threadid) {
    long tid = (long)threadid;
    int chunk_size = matrixSize / NUMTHRDS;
    int start = tid * chunk_size;
    int end = (tid == NUMTHRDS - 1) ? matrixSize : (tid + 1) * chunk_size;
    mm(start, end);
    pthread_exit(NULL);
}

void printResult(FILE *file) {
    int i, j;    
    for(i=0;i<matrixSize;i++){
        for(j=0;j<matrixSize;j++){
            fprintf(file, "%lf ", c[i][j]);
        }
        fprintf(file, "\n");
    }
}

int main(void) {
	int i, j, k;
	
	char *fname = "matrices_dev.dat"; //Change to matrices_large.dat for performance evaluation
	FILE *fh, *resultFile;

	pthread_t threads[NUMTHRDS];
    pthread_mutex_init(&mutex, NULL);
	
	resultFile = fopen("FG_result.txt", "w"); // Open a file to write the result
	printf("Start\n");
	fh = fopen(fname, "r");

	//First line indicates how many pairs of matrices there are and the matrix size
	fscanf(fh, "%d %d\n", &nmats, &matrixSize);

	//Dynamically create matrices of the size needed
	a = allocateMatrix();
	b = allocateMatrix();
	c = allocateMatrix();

	// printf("Loading %d pairs of square matrices of size %d from %s...\n", nmats, matrixSize, fname);
	for(k=0;k<nmats;k++){
		for(i=0;i<matrixSize;i++){
			for(j=0;j<matrixSize;j++){
				fscanf(fh, "%lf", &a[i][j]);
			}
		}
		for(i=0;i<matrixSize;i++){
			for(j=0;j<matrixSize;j++){
				fscanf(fh, "%lf", &b[i][j]);
			}
		}		
		// Create threads and execute matrix multiplication in parallel
        for (long t = 0; t < NUMTHRDS; t++) {
            pthread_create(&threads[t], NULL, threadMultiply, (void *)t);
        }
        // Join threads
        for (long t = 0; t < NUMTHRDS; t++) {
            pthread_join(threads[t], NULL);
        }
		printResult(resultFile); // Write result to file

	}
	fclose(fh);
	fclose(resultFile); // Close the result file
	pthread_mutex_destroy(&mutex);
	// Free memory
	free(*a);
	free(a);
	free(*b);
	free(b);
	free(*c);
	free(c);
	printf("Done.\n");
	return 0;
}

