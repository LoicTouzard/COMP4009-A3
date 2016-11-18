#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>

using namespace std;

// for debug use
void printBoard(int** board, int row, int col, char* processor)
{
  cout << "On processor : " << processor << endl;
  for (int i = 0; i < row; ++i)
  {
    for (int j = 0; j < col; ++j)
    {
      cout << board[i][j];
    }
    cout << endl;
  }
}


// ./assignment3 N k m FILENAME
// N size of the N*N matrix
// k number of evolutionary steps to do
// m output state in a file each m step
// m = 0 no output in file
// FILENAME output file name
int main(int argc, char *argv[]) {
  if (argc < 4)
  {
    cout << "not enough arguments" << endl << "Usage : ./assignment3 N k m FILENAME" << endl;
    return EXIT_FAILURE;
  }

  //initialization
  MPI_Init (&argc, &argv);
  
  int N = atoi(argv[1]); // size of the N*N board's matrix
  int k = atoi(argv[2]); // number of evolutionary steps to do
  int m = atoi(argv[3]); // m output state in a file each m step
  char* FILENAME= argv[4]; // output file name

  int rank; // rank of current process
  int p; // number of process
  char processor_name[MPI_MAX_PROCESSOR_NAME]; // name of the process
  int name_len;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Get_processor_name(processor_name, &name_len);
  // tags
  int TAG_SLICE_INIT = 1;

  int **board = NULL; // NxN
  int **slice = NULL; // NxN/p
  int normalSliceSize = N/p; // size of one slice
  int masterSliceSize = N-(N/p)*(p-1); // N = (p-1)*(N/P) + masterSliceSize   (to handle size not multiple of p)
  int processSliceSize = (rank==0)?masterSliceSize:normalSliceSize;

  if(rank == 0) // master
  {
    /** READ AND BUILT BOARD */

    // open the input file
    FILE* inputFile = fopen(FILENAME, "r"); // open the file and store the pointer
    if(inputFile == NULL)
    {
      cout << "Failed to open the input file : " << FILENAME << endl;
      return EXIT_FAILURE;
    }
    
    // instanciate full board
    // and fill the board from the file
    char *line = new char[N+5];
    board = new int*[N];
    for (int i = 0; i < N; ++i)
    {
      //read and allocate a line
      board[i] = new int[N];
      fgets(line, N+5, inputFile); 
      for (int j = 0; j < N; ++j)
      {
        board[i][j] = line[j]-'0';// int convertion from char with "-'0'"
      }
    }
    delete[] line;
    fclose(inputFile);
    //printBoard(board, N, N, "BOARD");


    /** SLICING **/    

    // slice for itself, master.
    slice = new int*[masterSliceSize];
    int rowCounter = 0;
    for (rowCounter = 0; rowCounter < masterSliceSize; ++rowCounter)
    {
      slice[rowCounter] = new int[N];
      for (int j = 0; j < N; ++j)
      {
        slice[rowCounter][j] = board[rowCounter][j];
      }
    }

    int nRequest = (N - masterSliceSize)*N;
    int indexRequest = 0;
    MPI_Request *reqs = new MPI_Request[nRequest];
    MPI_Status *stats = new MPI_Status[nRequest];
    // slice for slaves processes 
    for (int pRank = 1; pRank < p; ++pRank)
    {
      // send via MPI the slices to each process
      while(rowCounter < masterSliceSize+pRank*normalSliceSize)
      {
        for (int j = 0; j < N; ++j)
        {
          int value = board[rowCounter][j];
          MPI_Send(&value, 1, MPI_INT, pRank, TAG_SLICE_INIT, MPI_COMM_WORLD);//, &reqs[indexRequest++]);
        }
        ++rowCounter;
      }
    }
    //MPI_Waitall(nRequest, reqs, stats);
    delete[] reqs;
    delete[] stats;
  }
  else{ // slaves
    // receive a slice
    int nRequest = processSliceSize*N;
    int indexRequest = 0;
    MPI_Request *reqs = new MPI_Request[nRequest];
    MPI_Status *stats = new MPI_Status[nRequest];
    // allocate slice matrix and receive value
    slice = new int*[processSliceSize];
    for (int i = 0; i < processSliceSize; ++i)
    {
      slice[i] = new int[N];
      for (int j = 0; j < N; ++j)
      {
        int dest;
        MPI_Recv(&dest, 1, MPI_INT, 0, TAG_SLICE_INIT, MPI_COMM_WORLD, &stats[indexRequest++]);
        slice[i][j] = dest;
      }
    }
    //MPI_Waitall(nRequest, reqs, stats);
    delete[] reqs;
    delete[] stats;
  }

  // common to all process code
  MPI_Barrier(MPI_COMM_WORLD);

  usleep(10000*rank);
  printBoard(slice, processSliceSize, N, processor_name);



  // simulation loop
  for (int i = 0; i < k; ++i)
  {
    // compute new slice
  }



  // end and clean
  if(rank == 0) // master
  {
    // delete full matrix
    for (int i = 0; i < N; ++i)
    {
      delete[] board[i];
    }
    delete[] board;
    // delete master's slice
  }
  for (int i = 0; i < processSliceSize; ++i)
  {
    delete[] slice[i];
  }
  delete[] slice;

  MPI_Finalize();
  return EXIT_SUCCESS;
}
