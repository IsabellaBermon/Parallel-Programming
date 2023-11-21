#include <stdlib.h>
#include <stdio.h>
typedef struct Matrix
{
    double **a, **b, **c; 
} Matrix;

void mm(Matrix m, int matrixSize) {
	int i,j,k;
	double sum;

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

void printResult(Matrix m,int matrixSize) {
    FILE *resultFile;
	resultFile = fopen("CG_python.txt", "w"); 
    int i, j;    
    for(i=0;i<matrixSize;i++){
        for(j=0;j<matrixSize;j++){
            fprintf(resultFile, "%lf ", m.c[i][j]);
        }
        fprintf(resultFile, "\n");
    }
    
}