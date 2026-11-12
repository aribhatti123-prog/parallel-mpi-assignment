#include <iostream>
#include <string>
#include <cmath>
#include <mpi.h>

template <typename 

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

  float lastGrid[100][100] = {0.0}; //enhancing spacial locality, by ensuring data of the same type is together
  float currentGrid[100][100] = {0.0};
  float rankLastGrid[chunk][100] = {0.0};
  float rankCurrentGrid[chunk][100] = {0.0};
  float averageVal;
  float average(std::string neighboursList, float (*grid)[100], int i, int j, int* myrank, int* chunk);
  float valueN, valueE, valueS, valueW;
  double start;
  bool tempChange = true;

  if (myrank == 0) {
    start = MPI_Wtime();
    ierr = MPI_Scatter(&currentGrid, (chunk)*100, MPI_FLOAT, rankCurrentGrid, (chunk)*100, MPI_FLOAT, 0, MPI_COMM_WORLD);
  }

  lastGrid[50][50] = 10.0;
  lastGrid[40][60] = 7.2;
  lastGrid[70][25] = -1.2;

  while (tempChange==true) {
    tempChange = false;

    MPI_Wait(&lastSent, MPI_STATUS_IGNORE);
    for (int i=0; i<chunk; i++) {
      i_adjusted = i + (myrank * chunk);
      float* const zero = 0.0;
      //this is true across an entire run of j, so worth calculating, to reduce how often iterations across 
      //if statements occur
      if (0<i_adjusted<100) { //most common case, so first
        valueN = valueE = valueS = 0.0;
      }
      else if (i_adjusted==0) {
        valueW = 0.0;
      }
      else if (i_adjusted==100) {
        valueE = 0.0;
      }

      for (int j=0; j<100; j++) {
        //calculate the average value of 4 adjacent squares, bar corner
        averageVal = lastGrid[i_adjusted][j];

        if (0<j<100) {
          
        }

        //check if values have converged
        if (std::abs(averageVal-lastGrid[i][j])> 1e-4f) {
          tempChange = true;
        }
        else {
          rankCurrentGrid[i][j] = averageVal;
        }
      }
    }

    //copy grids over to last grid, and broadcast
    ierr = MPI_Iallgather(&rankCurrentGrid, chunk*100, MPI_FLOAT, lastGrid, 100*100, MPI_FLOAT, MPI_COMM_WORLD, &lastSent);
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

float average(std::string neighboursList, float (*grid)[100], int i, int j, int* myrank, int* chunk) {
  float sum = grid[i][j];
  i += *chunk * *myrank; //should eliminate the problem of halo data, as you are looking at the full grid anyway
  if (neighboursList.length()==4) {
    sum = grid[i][j] + grid[i-1][j] + grid[i+1][j] + grid[i][j-1] + grid[i][j+1];
  }
  else {
    for (int x=0; x<neighboursList.length(); x++) {
      if (neighboursList.substr(x,1)=="N") {
        sum += grid[i][j+1];
      }
      else if (neighboursList.substr(x,1)=="E") {
        sum += grid[i+1][j];
      }
      else if (neighboursList.substr(x,1)=="S") {
        sum += grid[i][j-1];
      }
      else if (neighboursList.substr(x,1)=="W") {
        sum += grid[i-1][j];
      }
    }
  }
  return sum/5.0;
}
