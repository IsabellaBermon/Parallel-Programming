// Matrix multiplication

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define NUMTHRDS 8;
typedef struct Matrix
{
    double **a, **b, **c; 
} Matrix;

Matrix matrixQueue[256];

int matrixSize;

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

void mm(void) {
	int i,j,k;
	double sum;
	// matrix multiplication
	for (i = 0; i < matrixSize; i++) {
		for (j = 0; j < matrixSize; j++) {
			sum = 0.0;
			// dot product
			for (k = 0; k < matrixSize; k++) {
				sum = sum + a[i][k] * b[k][j];
			}
			c[i][j] = sum;
		}
	}
}
/*
void printResult(void){
	int i, j;	
	for(i=0;i<matrixSize;i++){
		for(j=0;j<matrixSize;j++){
			printf("%lf ", c[i][j]);
		}
		printf("\n");
	}
}
*/
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
	int nmats;
	char *fname = "matrices_dev.dat"; //Change to matrices_large.dat for performance evaluation
	//FILE *fh;
	FILE *fh, *resultFile;
	pthread_t threads[NUMTHRDS];
	resultFile = fopen("CG_result.txt", "w"); // Open a file to write the result

	printf("Start\n");
	fh = fopen(fname, "r");

	//First line indicates how many pairs of matrices there are and the matrix size
	fscanf(fh, "%d %d\n", &nmats, &matrixSize);

	//Dynamically create matrices of the size needed

    

	printf("Loading %d pairs of square matrices of size %d from %s...\n", nmats, matrixSize, fname);
	for(k=0;k<nmats;k++){
        double **a = allocateMatrix();
        double **b = allocateMatrix();
        double **c = allocateMatrix();

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
        Matrix m = {
            .a = a,
            .b = b,
            .c =  c
        };
        matrixQueue[k] = m;

	}
    // Create threads and execute matrix multiplication in parallel
    for (long t = 0; t < NUMTHRDS; t++) {
        pthread_create(&threads[t], NULL, startThread, NULL);
    }



	fclose(fh);
	fclose(resultFile); // Close the result file

	// Free memory
	// free(*a);
	// free(a);
	// free(*b);
	// free(b);
	// free(*c);
	// free(c);
	printf("Done.\n");
	return 0;
}
