#!/bin/bash

# Nombre del archivo de salida
output_file="salida_total.txt"
# Número de veces que se ejecutará cada valor
num_iterations=20

# Itera sobre los valores de 1 a 32
for num in {1..32}
do
    # Imprime el valor correspondiente en el archivo
    echo "Valor de num: $num" >> "$output_file"

    # Inicializa la variable para almacenar el valor más pequeño
    min_time=9999999999  # Un valor inicial grande

    # Ejecuta el comando varias veces
    for ((i=1; i<=$num_iterations; i++))
    do
        # Ejecuta el comando y filtra la salida para obtener el valor "real"
        current_time=$( { time ./pFG $num; } 2>&1 | grep "real" | awk '{print $2}' )

        # Compara con el valor más pequeño hasta el momento
        if [[ "$current_time" < "$min_time" ]]; then
            min_time=$current_time
        fi
    done

    # Guarda el valor más pequeño en el archivo
    echo "Valor más pequeño: $min_time" >> "$output_file"

    # Añade un separador entre cada ejecución para mayor claridad
    echo "----------------------------------------" >> "$output_file"
done
