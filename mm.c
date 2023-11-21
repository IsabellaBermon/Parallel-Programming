#include <stdlib.h>
#include <stdio.h>
typedef struct Matrix
{
    double **a, **b, **c; 
} Matrix;

void mm(Matrix m, int matrixSize) {
	int i,j,k;
	double sum;
	// // Print matrix A
    // printf("Matrix A:\n");
    // for (i = 0; i < matrixSize; i++) {
    //     for (j = 0; j < matrixSize; j++) {
    //         printf("%lf ", m.a[i][j]);
    //     }
    //     printf("\n");
    // }
    // // Print matrix B
    // printf("Matrix B:\n");
    // for (i = 0; i < matrixSize; i++) {
    //     for (j = 0; j < matrixSize; j++) {
    //         printf("%lf ", m.b[i][j]);
    //     }
    //     printf("\n");
    // }
	// matrix multiplication
	for (i = 0; i < matrixSize; i++) {
		for (j = 0; j < matrixSize; j++) {
			sum = 0.0;
			// dot product
			for (k = 0; k < matrixSize; k++) {
				sum = sum + m.a[i][k] * m.b[k][j];
			}
            //fprintf(outputFile, "sum: [%f]\n", sum);
			m.c[i][j] = sum;
		}
	}

}