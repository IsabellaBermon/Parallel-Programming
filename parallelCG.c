// Matrix multiplication

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


int NUMTHRDS;
typedef struct Matrix
{
    double **a, **b, **c; 
} Matrix;

Matrix matrixQueue[256];
pthread_mutex_t mutexQueue;

int matrixCount = 0;
int matrixSize, nmats;

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
    pthread_mutex_lock(&mutexQueue);
    Matrix m = matrixQueue[matrixCount];
    matrixCount++;
    pthread_mutex_unlock(&mutexQueue);
	// matrix multiplication
	for (i = 0; i < matrixSize; i++) {
		for (j = 0; j < matrixSize; j++) {
			sum = 0.0;
			// dot product
			for (k = 0; k < matrixSize; k++) {
				sum = sum + m.a[i][k] * m.b[k][j];
			}
            printf("sum: [%f]\n", sum);
			m.c[i][j] = sum;
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
void* startThread(void* args) {
    while(1){
        mm();
        if(matrixCount == nmats){
            pthread_exit(NULL);
        }
    }
}
void printResult(FILE *file, Matrix m) {
    int i, j;    
    for(i=0;i<matrixSize;i++){
        for(j=0;j<matrixSize;j++){
            fprintf(file, "%lf ", m.c[i][j]);
        }
        fprintf(file, "\n");
    }
}
int compareFiles(FILE *file1, FILE *file2) {
    char line1[256], line2[256];
    int lineCount = 0;

    while (1) {
        // Leer una línea de cada archivo
        fgets(line1, sizeof(line1), file1);
        fgets(line2, sizeof(line2), file2);

        // Incrementar el contador de líneas
        lineCount++;

        // Comparar las líneas
        if (strcmp(line1, line2) != 0) {
            return 0;  // Archivos diferentes
        }

        // Verificar si hemos llegado al final de ambos archivos
        if (feof(file1) && feof(file2)) {
           
            return 1;  // Archivos idénticos
        } else if (feof(file1) || feof(file2)) {
            printf("Archivos de longitud diferente\n");
            return 0;  // Archivos de longitud diferente
        }
    }
}
int main(void) {
	int i, j, k;

	char *fname = "matrices_large.dat"; //Change to matrices_large.dat for performance evaluation
	//FILE *fh;
	FILE *fh, *resultFile,*originalFile;
	resultFile = fopen("CG_result.txt", "w"); // Open a file to write the result

	printf("Start\n");
	fh = fopen(fname, "r");

	//First line indicates how many pairs of matrices there are and the matrix size
	fscanf(fh, "%d %d\n", &nmats, &matrixSize);

	//Dynamically create matrices of the size needed
    printf("Ingrese el número de hilos ( 1 a %d): ",nmats);
    scanf("%d", &NUMTHRDS);
    
	pthread_t threads[NUMTHRDS];

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

    for (long t = 0; t < NUMTHRDS; t++) {
        pthread_join(threads[t], NULL);
    }

    for(long t = 0; t<nmats;t++){
        printResult(resultFile,matrixQueue[t]);
    }
    fclose(resultFile); // Close the result file
    resultFile = fopen("CG_result.txt", "r");
    originalFile = fopen("original_result.txt", "r");
    if (resultFile == NULL || originalFile == NULL) {
        perror("Error al abrir los archivos");
        exit(EXIT_FAILURE);
    }

    // Comparar los archivos
    if (compareFiles(resultFile, originalFile)) {
        printf("Contenido idéntico\n");
    } else {
        printf("Contenido diferente\n");
    }


	fclose(fh);
	
    pthread_mutex_destroy(&mutexQueue);
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
