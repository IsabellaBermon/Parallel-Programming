import threading
import ctypes
import numpy as np

cmm = ctypes.CDLL("./mm.so")

# Define Matrix structure
class Matrix(ctypes.Structure):
    _fields_ = [("a", ctypes.POINTER(ctypes.POINTER(ctypes.c_double))),
                ("b", ctypes.POINTER(ctypes.POINTER(ctypes.c_double))),
                ("c", ctypes.POINTER(ctypes.POINTER(ctypes.c_double)))]
# Define C data 
cmm.mm.restype = None
cmm.mm.argtypes = [Matrix, ctypes.c_int]
cmm.printResult.restype = None
cmm.printResult.argtypes = [Matrix, ctypes.c_int]

numthreads =32
nmats = 0
matrixSize = 0
matrixQueue =[]
matrixCount = 0
lock = threading.Lock()
def startThread():
    global matrixCount
    while(True):
        with lock:
            if(matrixCount <len(matrixQueue)):
                m = matrixQueue[matrixCount]
                matrixCount+=1
            else:
                break    
        cmm.mm(m, matrixSize)
        

with open("matrices_large.dat",'r') as file:
    line = file.readline()
    nmats = int(line.split()[0])
    matrixSize = int(line.split()[1])

    c = np.zeros((matrixSize, matrixSize), dtype=np.float64)
    rows = []
    for _ in range(0,nmats):
        for _ in range(matrixSize):
            line = file.readline()
            rows.append(list(map(np.float64, line.split())))

        a = np.array(rows,dtype=np.float64)    
        # print("A ",a)
        rows.clear()
        for _ in range(matrixSize):
            line = file.readline()
            rows.append(list(map(np.float64, line.split())))
        b = np.array(rows,dtype=np.float64)  
        # print("B ",b)
        rows.clear()
 
        # Convert matrices from numpy to matrices of C-compatible arrays
        a_c = (ctypes.POINTER(ctypes.c_double) * matrixSize)()
        b_c = (ctypes.POINTER(ctypes.c_double) * matrixSize)()
        c_c = (ctypes.POINTER(ctypes.c_double) * matrixSize)()

        for m in range(matrixSize):
            # Allocate memory for each row
            a_c[m] = (ctypes.c_double * matrixSize)()
            b_c[m] = (ctypes.c_double * matrixSize)()
            c_c[m] = (ctypes.c_double * matrixSize)()

            # Copy values from numpy arrays to C-compatible arrays
            for i in range(matrixSize):
                a_c[m][i] = a[m][i]  
                b_c[m][i] = b[m][i]   
                c_c[m][i] = ctypes.c_double() # Initialize c_c[m][i] as zeros

        matrix_struct = Matrix(a_c, b_c, c_c)
        matrixQueue.append(matrix_struct)


#Vamos a crear los hilos

#Se inicializan los hilos

#Se espera a que terminen
threads = []
for _ in range(numthreads):
    thread = threading.Thread(target=startThread)
    threads.append(thread)

# Iniciar hilos
for hilo in threads:
    hilo.start()

# Esperar a que todos los hilos terminen (opcional)
for hilo in threads:
    hilo.join()


# Inicializar una matriz para almacenar los resultados concatenados
result_matrix_c_combined = np.zeros((len(matrixQueue) * matrixSize, matrixSize), dtype=np.float64)

# Copiar los valores de cada matriz c de ctypes a la matriz combinada
for w in range(len(matrixQueue)):
    for i in range(matrixSize):
        for j in range(matrixSize):
            result_matrix_c_combined[w * matrixSize + i, j] = matrixQueue[w].c[i][j]

# Guardar la matriz combinada en un solo archivo
result_filename_combined = "python_result.txt"
np.savetxt(result_filename_combined, result_matrix_c_combined, fmt="%lf", delimiter=" ")
print(f"Result saved to {result_filename_combined}")