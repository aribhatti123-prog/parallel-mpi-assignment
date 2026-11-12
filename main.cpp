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
  double rankLastGrid[chunk][101] = {0.0};
  double rankCurrentGrid[chunk][101] = {0.0};
  double start;
  MPI_Request lastSent;

  double lastGrid[101][101] = {0.0};
  double currentGrid[101][101] = {0.0};
  bool tempChange = true;
  double averageVal;
  double average(std::string neighboursList, double (*grid)[101], int i, int j, int* myrank, int* chunk);

  if (myrank == 0) {
    start = MPI_Wtime();
    currentGrid[50][50] = 10.0;
    currentGrid[40][60] = 7.2;
    currentGrid[70][25] = -1.2;
    ierr = MPI_Scatter(&currentGrid, (chunk)*101, MPI_DOUBLE, rankCurrentGrid, (chunk)*101, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    lastGrid[50][50] = 10.0;
    lastGrid[40][60] = 7.2;
    lastGrid[70][25] = -1.2;
    ierr = MPI_Bcast(&lastGrid, 101*101, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }

  while (tempChange==true) {
    tempChange = false;

    MPI_Wait(&lastSent, MPI_STATUS_IGNORE);
    for (int i=0; i<chunk+1; i++) {
      for (int j=0; j<101; j++) {
        //calculate the average value of 4 adjacent squares, bar corner
        if (j==0) {
          if (i==0) {
            averageVal = average("ES", lastGrid, i, j, &myrank, &chunk);
          }
          else if (i==100) {
            averageVal = average("SW", lastGrid, i, j, &myrank, &chunk);
          }
          else {
            averageVal = average("ESW", lastGrid, i, j, &myrank, &chunk);
          }
        }
        else if (j==100) {
          if (i==0) {
            averageVal = average("NE", lastGrid, i, j, &myrank, &chunk);
          }
          else if (i==100) {
            averageVal = average("NW", lastGrid, i, j, &myrank, &chunk);
          }
          else {
            averageVal = average("NEW", lastGrid, i, j, &myrank, &chunk);
          }
        }
        else if (i==0) {
          averageVal = average("NES", lastGrid, i, j, &myrank, &chunk);
        }
        else if (i==100) {
          averageVal = average("NSW", lastGrid, i, j, &myrank, &chunk);
        }
        else {
          averageVal = average("NESW", lastGrid, i, j, &myrank, &chunk);
        }

        //check if values have converged
        if (std::abs(averageVal-lastGrid[i][j])> 1e-4f) {
          tempChange = true;
        }
        rankCurrentGrid[i][j] = averageVal;
      }
    }

    //copy grids over to last grid, and broadcast
    ierr = MPI_Iallgather(&rankCurrentGrid, chunk*101, MPI_DOUBLE, lastGrid, 101*101, MPI_DOUBLE, MPI_COMM_WORLD, &lastSent);
    ierr = MPI_Allreduce(&tempChange, &tempChange, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);
  }
  if (myrank==0) {
    double time = MPI_Wtime() - start;
    std::cout << "Equilibrium state reached in " << time <<"s. Centroid temp is " << lastGrid[55][55] << " K" << std::endl;
    std::cout << "The number of processors used was " << comm_size <<std::endl;
  }

  MPI_Finalize();
  return 0;
}

double average(std::string neighboursList, double (*grid)[101], int i, int j, int* myrank, int* chunk) {
  double sum = grid[i][j];
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
