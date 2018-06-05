#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void swap(int * arr, int i, int j)
{
  int t = arr[i];
  arr[i] = arr[j];
  arr[j] = t;
}

void bubblesort(int * arr, int n)
{
  int i, j;
  for (i = n-2; i >= 0; i--)
    for (j = 0; j <= i; j++)
      if (arr[j] > arr[j+1])
        swap(arr, j, j+1);
}

int * merge(int * arr1, int n1, int * arr2, int n2)
{
  int * result = (int *)malloc((n1 + n2) * sizeof(int));
  int i = 0;
  int j = 0;
  int k;
  for (k = 0; k < n1 + n2; k++) {
    if (i >= n1) {
      result[k] = arr2[j];
      j++;
    }
    else if (j >= n2) {
      result[k] = arr1[i];
      i++;
    }
    else if (arr1[i] < arr2[j]) {
      result[k] = arr1[i];
      i++;
    }
    else {
      result[k] = arr2[j];
      j++;
    }
  }
  return result;
}


int main(int argc, char ** argv)
{
  int size;
  int * list = NULL;
  int c, s;
  int * result;
  int o;
  int * other;
  int step;
  int p, processId;
  MPI_Status status;
  int i;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Comm_rank(MPI_COMM_WORLD, &processId);

  if (processId == 0) {
    // lee el tamaño que de datos que queremos ordenar
    size = atof(argv[1]);
    //Divide los datos entre los procesadores
    c = size/p; if (size%p) c++;
    // Crea el arreglo a ordenar con numeros random
    list = (int *)malloc(p*c * sizeof(int));
    for(int i = 0; i < size; i++)
        list[i] = rand();

    // pad para completar datos
    for (i = size; i < p*c; i++)
      list[i] = 0;
  }

  // broadcast size
  MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // compute result size
  c = size/p; if (size%p) c++;

  // scatter list
  result = (int *)malloc(c * sizeof(int));
  MPI_Scatter(list, c, MPI_INT, result, c, MPI_INT, 0, MPI_COMM_WORLD);
  free(list);
  list = NULL;

  // Realiza el bubblesort en cada procesador
  s = (size >= c * (processId+1)) ? c : size - c * processId;
  bubblesort(result, s);

  // Envia los datos que se ordenaron
  for (step = 1; step < p; step = 2*step) {
    if (processId % (2*step)!=0) {
      MPI_Send(result, s, MPI_INT, processId-step, 0, MPI_COMM_WORLD);
      break;
    }
    if (processId+step < p) {
      o = (size >= c * (processId+2*step)) ? c * step : size - c * (processId+step);
      // Recibe los resultados de cada procesador
      other = (int *)malloc(o * sizeof(int));
      MPI_Recv(other, o, MPI_INT, processId+step, 0, MPI_COMM_WORLD, &status);
      // Se hace el merge entre los elementos ordenados de cada procesador
      list = merge(result, s, other, o);
      free(result);
      free(other);
      result = list;
      s = s + o;
    }
  }

  // Devuelve la cadena ordenada
  if (processId == 0) {
    for(int i=0; i<s; i++)
        printf("%d " ,result[i]);
  }

  MPI_Finalize();
  return 0;
}
