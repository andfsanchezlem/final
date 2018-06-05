mpicc bubble_mpi.c -o bubble_mpi

for processor in 2 3 4 5 6 8
do
    for elements in 10 100 1000 10000 100000
    do

      echo "------------------------------------------">> mediciones_mpi.txt
      echo "Usando " $processor " procesadores y" $elements "elements">> mediciones_mpi.txt
      { time mpirun -np $processor ./bubble_mpi $elements >/dev/null 2>&1; } 2>> mediciones_mpi.txt

    done
  done
