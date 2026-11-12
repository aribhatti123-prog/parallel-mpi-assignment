#include <iostream>
#include <string>
#include <cmath>
#include <mpi.h>

int main() {

  float lastGrid[102][102] = {0.0};
  float currentGrid[100][100] = {0.0};
  float averageVal;
  float maxDiff = 0.0;
  bool tempChange = true;

  currentGrid[50][50] = 10.0;
  currentGrid[40][60] = 7.2;
  currentGrid[70][25] = -1.2;

  double start = MPI_Wtime();

  while (tempChange==true) {
    tempChange = false;
    //copy grids over
    maxDiff = 0.0;
    for (int i=0; i<100; i++) {
      for (int j=0; j<100; j++) {
        lastGrid[i+1][j+1] = currentGrid[i][j];
      }
    }
    for (int i=0; i<100; i++) {
      for (int j=0; j<100; j++) {
        //calculate the average value of 4 adjacent squares, bar corner
        averageVal = lastGrid[i+1][j] + lastGrid[i+1][j+1] + lastGrid[i+1][j+2] + lastGrid[i][j+1] + lastGrid[i+2][j+1];
        averageVal /= 5.0;

        //check if values have converged
        maxDiff = std::fmax(maxDiff, std::fabs(averageVal - lastGrid[i+1][j+1]));
        currentGrid[i][j] = averageVal;
      }
    }
    if (maxDiff>1e-4f) {
      tempChange = true;
    }
  }

  double time = MPI_Wtime() - start;
  std::cout << "Equilibrium state reached in " << time <<" s. Centroid temp is " << currentGrid[55][55] << " C" << std::endl;

  return 0;
}
