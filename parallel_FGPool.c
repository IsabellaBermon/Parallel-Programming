
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct MatrixMultiplyTask {
    int start, end;
} MatrixMultiplyTask;

MatrixMultiplyTask taskQueue[256];
int taskCount = 0;
pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

double **a, **b, **c;
int matrixSize, nmats;
pthread_mutex_t mutex;

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
    printf("Task submitted: [%d, %d], taskCount: %d\n", task.start, task.end, taskCount);
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

void mm(int start, int end) {
    int i, j, k;
    double sum;
    for (i = start; i < end; i++) {
        for (j = 0; j < matrixSize; j++) {
            sum = 0.0;
            for (k = 0; k < matrixSize; k++) {
                sum = sum + a[i][k] * b[k][j];
                
            }
            // printf("sum: [%f]\n", sum);
            pthread_mutex_lock(&mutex);
            c[i][j] = sum;
            pthread_mutex_unlock(&mutex);
        }
    }
}

void* startThread(void* args) {
    while (1) {
        
        MatrixMultiplyTask task;

        pthread_mutex_lock(&mutexQueue);
        while (taskCount == 0 && !end) {
            printf("Thread waiting for tasks...\n");
            pthread_cond_wait(&condQueue, &mutexQueue);
          

        }
        if (end) {
            printf("Thread received exit signal, exiting...\n");
            pthread_mutex_unlock(&mutexQueue);

            pthread_exit(NULL);
        }

        task = taskQueue[0];
        int i;
        for (i = 0; i < taskCount - 1; i++) {
            taskQueue[i] = taskQueue[i + 1];
        }
        taskCount--;
        pthread_mutex_unlock(&mutexQueue);

           

        mm(task.start, task.end);
        pthread_mutex_lock(&mutex);
        checkMultiply++;
        pthread_mutex_unlock(&mutex);

    }

}
void printResult(FILE *file) {
    int i, j;
    for (i = 0; i < matrixSize; i++) {
        for (j = 0; j < matrixSize; j++) {
            fprintf(file, "%lf ", c[i][j]);
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

int main(int argc, char *argv[]) {

    NUMTHRDS = atoi(argv[1]);
  
    

    pthread_t threads[NUMTHRDS];
    // pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);

    char *fname = "matrices_large.dat"; //Change to matrices_large.dat for performance evaluation
    FILE *fh, *resultFile,*originalFile;


    resultFile = fopen("FG_result.txt", "w"); // Open a file to write the result
	printf("Start\n");
	fh = fopen(fname, "r");

    // First line indicates how many pairs of matrices there are and the matrix size
    fscanf(fh, "%d %d", &nmats, &matrixSize);

    //Dynamically create matrices of the size needed
	a = allocateMatrix();
	b = allocateMatrix();
	c = allocateMatrix();

    // Create threads and execute matrix multiplication in parallel
    for (long t = 0; t < NUMTHRDS; t++) {
        pthread_create(&threads[t], NULL, startThread, NULL);
    }

    // Read matrices from file
    for (int k = 0; k < nmats; k++) {
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
        int chunk_size = matrixSize / NUMTHRDS;
        for (long t = 0; t < NUMTHRDS; t++) {
            MatrixMultiplyTask task = {
                .start = t * chunk_size,
                .end = (t == NUMTHRDS - 1) ? matrixSize : (t + 1) * chunk_size
            };
            submitTask(task);
        }       
        while (1)
        {
            if(checkMultiply == NUMTHRDS){
                checkMultiply=0;
                break;
            }
        }
        printResult(resultFile); 
    }


    for(int k = 0; k< NUMTHRDS; k++){
        taskCount = 0;
        end = 1;
        pthread_cond_signal(&condQueue);
    }
    
    // Join threads
    for (long t = 0; t < NUMTHRDS; t++) {
        pthread_join(threads[t], NULL);
    }

   // Write result to file
    fclose(resultFile); // Close the result file

    // Check results ---------------------------------
    resultFile = fopen("FG_result.txt", "r");
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
    fclose(resultFile); // Close the result file
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);

    // // Free memory

    free(*a);
    free(*b);
    free(*c);
    
    free(a);
    free(b);
    free(c);

    printf("Done.\n");
    return 0;
}
