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
  cout << flush;
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
  int TAG_SLICE_INIT = 1, TAG_TOP_SLICE = 2, TAG_BOTTOM_SLICE = 3;

  int **board = NULL; // NxN
  int **slice, **sliceNew = NULL; // NxN/p  ; sliceNew is the result after computation
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
    sliceNew = new int*[masterSliceSize];
    int rowCounter = 0;
    for (rowCounter = 0; rowCounter < masterSliceSize; ++rowCounter)
    {
      slice[rowCounter] = new int[N];
      sliceNew[rowCounter] = new int[N];
      for (int j = 0; j < N; ++j)
      {
        slice[rowCounter][j] = board[rowCounter][j];
      }
    }

    int nRequest = (N - masterSliceSize);
    int indexRequest = 0;
    MPI_Request *reqs = new MPI_Request[nRequest];
    MPI_Status *stats = new MPI_Status[nRequest];
    // slice for slaves processes 
    for (int pRank = 1; pRank < p; ++pRank)
    {
      // send via MPI the slices to each process
      while(rowCounter < masterSliceSize+pRank*normalSliceSize)
      {
        MPI_Isend(board[rowCounter], N, MPI_INT, pRank, TAG_SLICE_INIT, MPI_COMM_WORLD, reqs+indexRequest);
        //MPI_Send(board[rowCounter], N, MPI_INT, pRank, TAG_SLICE_INIT, MPI_COMM_WORLD);//, &reqs[indexRequest++]);
        ++rowCounter;
        ++indexRequest;
      }
    }
    MPI_Waitall(nRequest, reqs, stats);
    delete[] reqs;
    delete[] stats;
  }
  else{ // slaves
    // receive a slice
    int indexRequest = 0;
    MPI_Request *reqs = new MPI_Request[processSliceSize];
    MPI_Status *stats = new MPI_Status[processSliceSize];
    // allocate slice matrix and receive value
    slice = new int*[processSliceSize];
    sliceNew = new int*[processSliceSize];
    for (int i = 0; i < processSliceSize; ++i)
    {
      slice[i] = new int[N];
      sliceNew[i] = new int[N];
      MPI_Irecv(slice[i], N, MPI_INT, 0, TAG_SLICE_INIT, MPI_COMM_WORLD, reqs+indexRequest);
      ++indexRequest;
    }
    MPI_Waitall(processSliceSize, reqs, stats);
    delete[] reqs;
    delete[] stats;
  }

  // common to all process code
  MPI_Barrier(MPI_COMM_WORLD); // we start when everyone get its matrix ready
  usleep(rank*10000); // delay to debug in order
  printBoard(slice, processSliceSize, N, processor_name);
  MPI_Barrier(MPI_COMM_WORLD); // we start when everyone get its matrix ready


  // simulation variable
  int* topRow = NULL;
  int* bottomRow = NULL;
  int topNeighbour = 1, bottomNeighbour = -1; // none
  if(rank != 0){
    topRow = new int[N];
    topNeighbour = rank-1;
  }
  if(rank != p-1){
    bottomRow = new int[N];
    bottomNeighbour = rank+1;
  }
  
  // simulation loop
  for (int ki = 0; ki < 1; ++ki)
  {
    // MPI_Isend(board[ro, N, MPI_INT, pRank, TAG_SLICE_INIT, MPI_COMM_WORLD, reqs+indexRequest);
    // MPI_Irecv(slice[i], N, MPI_INT, 0, TAG_SLICE_INIT, MPI_COMM_WORLD, reqs+indexRequest);


    // row trades
    if(rank == 0) cout << "Computation of step " << ki << endl;
    int nRequest = (rank == 0 || rank == p-1)?2:4;
    int indexRequest = 0;
    MPI_Request *reqs = new MPI_Request[nRequest];
    MPI_Status *stats = new MPI_Status[nRequest];
    if(rank != 0){
      MPI_Isend(slice[0], N, MPI_INT, topNeighbour, TAG_BOTTOM_SLICE, MPI_COMM_WORLD, reqs); // receiver's bottom
      MPI_Irecv(topRow, N, MPI_INT, topNeighbour, TAG_TOP_SLICE, MPI_COMM_WORLD, reqs+1);
      indexRequest+=2;
    }
    if(rank != p-1){
      MPI_Isend(slice[processSliceSize-1], N, MPI_INT, bottomNeighbour, TAG_TOP_SLICE, MPI_COMM_WORLD, reqs+indexRequest); // receiver's top
      MPI_Irecv(bottomRow, N, MPI_INT, bottomNeighbour, TAG_BOTTOM_SLICE, MPI_COMM_WORLD, reqs+indexRequest+1);
      indexRequest += 2;
    }
    MPI_Waitall(nRequest, reqs, stats);

    usleep(rank*10000); // delay to debug in order
    if(rank != 0) printBoard(&topRow, 1, N, processor_name);
    if(rank != p-1) printBoard(&bottomRow, 1, N, processor_name);
    MPI_Barrier(MPI_COMM_WORLD); // we start when everyone get its matrix ready
    /*
    if(rank%2 == 1){// changing tasks order for synchronization and prevent deadlocks
      // receive topRow
      if(rank != 0) MPI_Recv(topRow, N, MPI_INT, topNeighbour, TAG_TOP_SLICE, MPI_COMM_WORLD, stats);
      if(rank != 0) cout << processor_name << " recv top" << flush << endl;
      // send topRow
      if(rank != 0) MPI_Send(slice[0], N, MPI_INT, topNeighbour, TAG_BOTTOM_SLICE, MPI_COMM_WORLD); // receiver's bottom
      if(rank != 0) cout << processor_name << " send top" << flush << endl;
      // receive bottom row
      if(rank != p-1) MPI_Recv(bottomRow, N, MPI_INT, bottomNeighbour, TAG_BOTTOM_SLICE, MPI_COMM_WORLD, stats);
      if(rank != p-1) cout << processor_name << " recv bot" << flush << endl;
      // send bottom row
      if(rank != p-1) MPI_Send(slice[processSliceSize-1], N, MPI_INT, topNeighbour, TAG_TOP_SLICE, MPI_COMM_WORLD); // receiver's top
      if(rank != p-1) cout << processor_name << " send bot" << flush << endl;
    }
    else{ // use of symetric order
      // send bottom row
      if(rank != p-1) MPI_Send(slice[processSliceSize-1], N, MPI_INT, topNeighbour, TAG_TOP_SLICE, MPI_COMM_WORLD); // receiver's top
      if(rank != p-1) cout << processor_name << " send bot" << flush << endl;
      // receive bottom row
      if(rank != p-1) MPI_Recv(bottomRow, N, MPI_INT, bottomNeighbour, TAG_BOTTOM_SLICE, MPI_COMM_WORLD, stats);
      if(rank != p-1) cout << processor_name << " recv bot" << flush << endl;
      // send topRow
      if(rank != 0) MPI_Send(slice[0], N, MPI_INT, topNeighbour, TAG_BOTTOM_SLICE, MPI_COMM_WORLD); // receiver's bottom
      if(rank != 0) cout << processor_name << " send top" << flush << endl;
      // receive topRow
      if(rank != 0) MPI_Recv(topRow, N, MPI_INT, topNeighbour, TAG_TOP_SLICE, MPI_COMM_WORLD, stats);
      if(rank != 0) cout << processor_name << " recv top" << flush << endl;
    }
    */
    delete[] stats;
    delete[] reqs;

    /*

    // compute new slice
    for (int i = 0; i < processSliceSize; ++i)
    {
      for (int j = 0; j < N; ++j)
      {
        // analyse of the cell : slice[i][j]
        int sum = 0; // sum of the 3*3 square
        for (int l = ((j==0)?j:j-1); l <= j+1 && l < N; ++l) // for each existing col of this square
        {
          // row before i
          if(i==0 && rank != 0) sum += topRow[l];
          else sum += slice[i-1][l];
          // row i
          if(l != j) sum += slice[i][l]; // doens't add itself in the sum
          // row after i
          if(i==(processSliceSize-1) && rank != p-1) sum += bottomRow[l];
          else sum += slice[i+1][l];
        }
        // analyse the sum
        if(sum <= 1) sliceNew[i][j] = 0; // starvation
        if(sum == 3) sliceNew[i][j] = 1; // birth
        if(sum >= 4) sliceNew[i][j] = 0; // overpopulation
      }
    }
    
   
    if(m != 0 && ki != 0 && ki%m == 0){ // log int file condition
      if(rank == 0){
        // master
        for (int i = 1; i < p; ++i)
        {
          // receive and update the board
        }
      }
      else{
        // slaves
      }
    }

    */
    MPI_Barrier(MPI_COMM_WORLD); // we start when everyone get its matrix ready

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
    delete[] sliceNew[i];
  }
  delete[] slice;
  delete[] sliceNew;

  MPI_Finalize();
  return EXIT_SUCCESS;
}
