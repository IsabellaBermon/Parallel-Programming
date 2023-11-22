#!/bin/bash

# Nombre del archivo de salida
output_file="salida_total2.txt"
# Número de veces que se ejecutará cada valor
num_iterations=20

# Itera sobre los valores de 1 a 32
for num in {1..32}
do
    # Imprime el valor correspondiente en el archivo
    # echo "Valor de num: $num" >> "$output_file"

    # Inicializa la variable para almacenar la suma de tiempos
    total_time=0

    # Ejecuta el comando varias veces
    for ((i=1; i<=$num_iterations; i++))
    do
        # Ejecuta el comando y filtra la salida para obtener el valor "real"
        current_time=$( { time python3 multPython.py $num; } 2>&1 | grep "real" | awk '{print $2}' )

        # Elimina la parte "0m" y cualquier otro carácter no numérico
        current_time=$(echo "$current_time" | tr -cd '0-9\n')

        # Suma los tiempos
        total_time=$(echo "$total_time + $current_time" | bc)
    done

    # Calcula el promedio dividiendo la suma por el número de iteraciones
    average_time=$(echo "$total_time / $num_iterations" | bc)

    # Guarda el promedio en el archivo sin el punto decimal
    echo "${average_time%.*}" >> "$output_file"

    # Añade un separador entre cada ejecución para mayor claridad
    # echo "----------------------------------------" >> "$output_file"
done
