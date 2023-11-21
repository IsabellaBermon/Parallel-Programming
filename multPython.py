import threading
import ctypes
import numpy as np

cmm = ctypes.CDLL("./mm.so")

# Definir la estructura Matrix
class Matrix(ctypes.Structure):
    _fields_ = [("a", ctypes.POINTER(ctypes.POINTER(ctypes.c_double))),
                ("b", ctypes.POINTER(ctypes.POINTER(ctypes.c_double))),
                ("c", ctypes.POINTER(ctypes.POINTER(ctypes.c_double)))]
cmm.mm.restype = None
cmm.mm.argtypes = [Matrix, ctypes.c_int]

cmm.printResult.restype = None
cmm.printResult.argtypes = [Matrix, ctypes.c_int]
# Función para convertir una matriz de NumPy a una estructura Matrix de ctypes


# Convertir matrices de numpy a matrices de C




nmats = 0
matrixSize = 0
matrixQueue =[]
with open("matrices_dev.dat",'r') as file:
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
        rows.clear()
        for _ in range(matrixSize):
            line = file.readline()
            rows.append(list(map(np.float64, line.split())))
        b = np.array(rows,dtype=np.float64)    
        rows.clear()
        # Convertir matrices de numpy a matrices de C
 
        a_c = (ctypes.POINTER(ctypes.c_double) * matrixSize)()
        b_c = (ctypes.POINTER(ctypes.c_double) * matrixSize)()
        c_c = (ctypes.POINTER(ctypes.c_double) * matrixSize)()
        for m in range(matrixSize):
            a_c[m] = a[m].ctypes.data_as(ctypes.POINTER(ctypes.c_double))
            b_c[m] = b[m].ctypes.data_as(ctypes.POINTER(ctypes.c_double))
            c_c[m] = c[m].ctypes.data_as(ctypes.POINTER(ctypes.c_double))
        matrix_struct = Matrix(a_c, b_c, c_c)
        matrixQueue.append(matrix_struct)


print("tamaño",len(matrixQueue))
# # Crear la estructura Matrix
# matrix_struct = Matrix(a_c, b_c, c_c)

# # Llamar a la función mm
for w in range(len(matrixQueue)):
    cmm.mm(matrixQueue[w], matrixSize)
    #cmm.printResult(matrixQueue[w], matrixSize)
# Inicializar una matriz para almacenar los resultados concatenados
result_matrix_c_combined = np.zeros((len(matrixQueue) * matrixSize, matrixSize), dtype=np.float64)

# Copiar los valores de cada matriz c de ctypes a la matriz combinada
for w in range(len(matrixQueue)):
    for i in range(matrixSize):
        for j in range(matrixSize):
            result_matrix_c_combined[w * matrixSize + i, j] = matrixQueue[w].c[i][j]

# Guardar la matriz combinada en un solo archivo
result_filename_combined = "result_combined.txt"
np.savetxt(result_filename_combined, result_matrix_c_combined, fmt="%lf", delimiter=" ")
print(f"Combined result saved to {result_filename_combined}")
# # Imprimir el resultado

# print("Matrix C:")
# print(c)