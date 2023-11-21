# Parallel-Programming
Parallel Programming Assignment

# Documento original de multiplicación
gcc -pthread -o original_mult original_mult.c
./original_mult
-> genera : original_result

# Multiplicación paralela Coarse-grain
gcc -pthread -o parallel_CG parallel_CG.c
./parallel_CG
-> genera : CG_result

# Multiplicación paralela Fine-grain
gcc -pthread -o parallel_FG parallel_FG.c
./parallel_FG
-> genera : FG_result

# Compilar C para python
gcc -c -fPIC mm.c -o mm.o
gcc -shared mm.o -o mm.so
