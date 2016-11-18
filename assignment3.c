#include "mpi.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

  FILE *f = fopen("mpi_output_test.log","w");

  int rank, wsize;

  MPI_Status status;
  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &wsize);

  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;

  MPI_Get_processor_name(processor_name, &name_len);

  printf("Hello Worlld!, I am processor with rank %d on node %s.\n",rank,processor_name);
  fprintf(f, "Hello World!, I am processor with rank %d on node %s.\n",rank,processor_name);

  fclose(f);

  MPI_Finalize();
  return 0;
}
