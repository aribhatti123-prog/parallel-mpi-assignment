#include <iostream>
#include <string>
#include <cmath>
#include <mpi.h>

int main(int argc, char **argv) {

  int ierr;
  MPI_Init(&argc, &argv);
  int myrank;
  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  int comm_size;
  ierr = MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  int chunk = 100/comm_size;
  MPI_Request lastSent = MPI_REQUEST_NULL;
  int i_adjusted;
  int displacements[comm_size];
  long unsigned int recvCount[comm_size] = {chunk*100*sizeof(float)*sizeof(float)};

  for (int i=0; i<comm_size; i++) {
    displacements[i] = (chunk*i) + 1;
  }

  float lastGrid[102][102] = {0.0}; //enhancing spacial locality, by ensuring data of the same type is together
  float currentGrid[100][100] = {0.0};
  float rankCurrentGrid[chunk][100] = {0.0};
  float averageVal;
  float maxDiff = 0;
  double start;
  bool tempChange = true;

  if (myrank == 0) {
    start = MPI_Wtime();
    ierr = MPI_Scatter(&currentGrid, (chunk)*100, MPI_FLOAT, rankCurrentGrid, (chunk)*100, MPI_FLOAT, 0, MPI_COMM_WORLD);
  }

  lastGrid[51][51] = 10.0;
  lastGrid[41][61] = 7.2;
  lastGrid[71][26] = -1.2;

  while (tempChange==true) {
    tempChange = false;
    maxDiff = 0;

    MPI_Wait(&lastSent, MPI_STATUS_IGNORE);
    for (int i=0; i<chunk; i++) {
      i_adjusted = i + (myrank * chunk);

      for (int j=0; j<100; j++) {
        //calculate the average value of 4 adjacent squares, bar corner
        averageVal = lastGrid[i_adjusted][j+1] + lastGrid[i_adjusted+1][j+1] + lastGrid[i_adjusted+2][j+1]
                     + lastGrid[i_adjusted+1][j] + lastGrid[i_adjusted][j+2];
        averageVal = averageVal / 5.0;
        maxDiff = std::fmax(maxDiff, std::fabs(averageVal - lastGrid[i_adjusted+1][j+1]));
        rankCurrentGrid[i][j] = averageVal;
      }
    }
    if (maxDiff > 1e-4f) {
      tempChange = true;
    }

    //copy grids over to last grid, and broadcast
    ierr = MPI_Iallgatherv(&rankCurrentGrid, chunk*100, MPI_FLOAT, lastGrid, recvCount, displacements, MPI_FLOAT, MPI_COMM_WORLD, &lastSent);
    ierr = MPI_Allreduce(&tempChange, &tempChange, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);
  }

  MPI_Wait(&lastSent, MPI_STATUS_IGNORE);
  if (myrank==0) {
    double time = MPI_Wtime() - start;
    std::cout << "Equilibrium state reached in " << time <<"s. Centroid temp is " << lastGrid[55][55] << " K" << std::endl;
    std::cout << "The number of processors used was " << comm_size <<std::endl;
  }

  MPI_Finalize();
  return 0;
}
