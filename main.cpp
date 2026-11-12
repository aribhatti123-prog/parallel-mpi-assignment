#include <iostream>
#include <string>
#include <cmath>
#include <mpi.h>
#include <fstream>

int main(int argc, char **argv) {

  int ierr;
  MPI_Init(&argc, &argv);
  int myrank;
  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  int comm_size;
  ierr = MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  int chunk = 100/comm_size;
  int i_adjusted;

  float lastGrid[102][102] = {0.0}; //enhancing spacial locality, by ensuring data of the same type is together
  float currentGrid[100][100] = {0.0};
  float rankCurrentGrid[chunk][100] = {0.0};
  float averageVal;
  float maxDiff = 0;
  double start;
  bool tempChange = true;

  std::ofstream fout;
  for (int n=0; n<10; n++){ //runs each iteration of the code 10 times for each no. processor

  if (myrank == 0) {
    start = MPI_Wtime();
  }

  lastGrid[51][51] = 10.0;
  lastGrid[41][61] = 7.2;
  lastGrid[71][26] = -1.2;

  while (tempChange==true) {
    tempChange = false;
    maxDiff = 0;

    for (int i=0; i<chunk; i++) {
      i_adjusted = i + (myrank * chunk);

      for (int j=0; j<100; j++) {
        //calculate the average value of 4 adjacent squares, bar corner
        averageVal = lastGrid[i_adjusted][j+1] + lastGrid[i_adjusted+1][j+1] + lastGrid[i_adjusted+2][j+1]
                     + lastGrid[i_adjusted+1][j] + lastGrid[i_adjusted+1][j+2];
        averageVal /= 5.0;
        maxDiff = std::fmax(maxDiff, std::fabs(averageVal - lastGrid[i_adjusted+1][j+1]));
        rankCurrentGrid[i][j] = averageVal;
      }
    }
    if (maxDiff > 1e-4f) {
      tempChange = true;
    }

    //copy grids over to last grid, and broadcast
    ierr = MPI_Allgather(&rankCurrentGrid, chunk*100, MPI_FLOAT, currentGrid, chunk*100, MPI_FLOAT, MPI_COMM_WORLD);
    ierr = MPI_Allreduce(MPI_IN_PLACE, &tempChange, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);
    for (int x=0; x<100; x++) {
      for (int y=0; y<100; y++) {
        lastGrid[x+1][y+1] = currentGrid[x][y];
      }
    }
  }

  if (myrank==0) {
    double time = MPI_Wtime() - start;
    std::cout << "Equilibrium state reached in " << time <<"s. Centroid temp is " << lastGrid[56][56] << " C" << std::endl;
    std::cout << "The number of processors used was " << comm_size <<std::endl;
    std::ofstream fout("output.csv", std::ios::app);
    fout << comm_size << " , " << lastGrid[56][56] << " , " << time << "\n";
    fout.close();
  }

  }

  MPI_Finalize();
  return 0;
}
