
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
typedef struct MatrixMultiplyTask {
    int start, end;
} MatrixMultiplyTask;

typedef struct MatrixResult
{
    double **a, **b,**c;
}MatrixResult;


MatrixMultiplyTask taskQueue[256];
MatrixResult taskResult[256];
int taskCount = 0;


pthread_mutex_t mutex;
pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

int matrixSize, nmats;

int NUMTHRDS= 2;
int checkMultiply = 0;
int end =0;

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

void submitTask(MatrixMultiplyTask task) {
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = task;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

void mm(MatrixMultiplyTask tse, MatrixResult mr) {
    int i, j, k;
    double sum;
    for (i = tse.start; i < tse.end; i++) {
        pthread_mutex_lock(&mutex);
        for (j = 0; j < matrixSize; j++) {
            sum = 0.0;
            for (k = 0; k < matrixSize; k++) {
                sum = sum + mr.a[i][k] * mr.b[k][j];

            }
            mr.c[i][j] = sum;
        }
        pthread_mutex_unlock(&mutex);
    }
}

void* startThread(void* args) {
    pthread_mutex_lock(&mutexQueue);
    MatrixMultiplyTask tse = taskQueue[taskCount];
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);
    int taskCountResult= 0;
    while (1) {
        MatrixResult rs ;
        pthread_mutex_lock(&mutexQueue);
        if(taskCountResult < nmats){

            rs = taskResult[taskCountResult];
            taskCountResult++;
        }else{
            pthread_mutex_unlock(&mutexQueue);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&mutexQueue);
        
        mm(tse,rs);

    }
}

void printResult(FILE *file,MatrixResult m) {
    int i, j;
    for (i = 0; i < matrixSize; i++) {
        for (j = 0; j < matrixSize; j++) {
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
            // printf("Archivos de longitud diferente\n");
            return 0;  // Archivos de longitud diferente
        }
    }
}

void freeMatrix(double **matrix) {
    free(matrix[0]);  // Free the memory block that contains the matrix values
    free(matrix);      // Free the array of pointers
}
int main(int argc, char *argv[]) {

    NUMTHRDS = atoi(argv[1]);
    pthread_t threads[NUMTHRDS];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);

    char *fname = "matrices_large.dat"; 
    FILE *fh, *resultFile,*originalFile;

    // resultFile = fopen("FG_result2.txt", "w"); // Open a file to write the result
	fh = fopen(fname, "r");

    // First line indicates how many pairs of matrices there are and the matrix size
    fscanf(fh, "%d %d", &nmats, &matrixSize);

    // Dynamically create matrices of the size needed

    // Create threads and execute matrix multiplication in parallel
    // Read matrices from file

    int chunk_size = matrixSize / NUMTHRDS;
    for (long t = 0; t < NUMTHRDS; t++) {
        MatrixMultiplyTask task = {
            .start = t * chunk_size,
            .end = (t == NUMTHRDS - 1) ? matrixSize : (t + 1) * chunk_size
        };
        taskQueue[t] = task;
    }
    for (int k = 0; k < nmats; k++) {
        double **a = allocateMatrix();
        double **b = allocateMatrix();
        double **c = allocateMatrix();
        for (int i = 0; i < matrixSize; i++) {
            for (int j = 0; j < matrixSize; j++) {
                fscanf(fh, "%lf", &a[i][j]);
            }
        }
        for (int i = 0; i < matrixSize; i++) {
            for (int j = 0; j < matrixSize; j++) {
                fscanf(fh, "%lf", &b[i][j]);
            }
        }        
     
        // Split matrix multiplication into tasks and submit to the queue
       
        MatrixResult cr = {
            .a=a,
            .b=b,
            .c=c
        };
        taskResult[k]=cr;


       
    }

    for (long t = 0; t < NUMTHRDS; t++) {
        pthread_create(&threads[t], NULL, startThread, NULL);
    }

    
    // Join threads
    for (long t = 0; t < NUMTHRDS; t++) {
        pthread_join(threads[t], NULL);
    }
    // for(long t = 0; t<nmats;t++){
    //     printResult(resultFile,taskResult[t]);
    // }
   // Write result to file
    // fclose(resultFile); // Close the result file

    // // // Check results ---------------------------------
    // resultFile = fopen("FG_result2.txt", "r");
    // originalFile = fopen("original_result.txt", "r");
    // if (resultFile == NULL || originalFile == NULL) {
    //     perror("Error al abrir los archivos");
    //     exit(EXIT_FAILURE);
    // }
    // // Comparar los archivos
    // if (compareFiles(resultFile, originalFile)) {
    //     printf("Contenido idéntico\n");
    // } else {
    //     printf("Contenido diferente\n");
    // }
    // fclose(resultFile);
    // fclose(originalFile);
    // -----------------------------------------------

    fclose(fh);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);

    // Free memory
    for (int t = 0; t < nmats; t++) {
        freeMatrix(taskResult[t].a);
        freeMatrix(taskResult[t].b);
        freeMatrix(taskResult[t].c);
    }
  

    return 0;
}