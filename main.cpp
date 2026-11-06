#include <iostream>
#include <string>
#include <cmath>

int main() {

  double lastGrid[101][101] = {0};
  double currentGrid[101][101] = {0};
  bool tempChange = true;
  double averageVal;
  double average(std::string neighboursList, double (*grid)[101], int i, int j);

  currentGrid[50][50] = 10.0;
  currentGrid[40][60] = 7.2;
  currentGrid[70][25] = -1.2;

  while (tempChange==true) {
    tempChange = false;
    //copy grids over
    for (int i=0; i<101; i++) {
      for (int j=0; j<101; j++) {
        lastGrid[i][j] = currentGrid[i][j];
      }
    }
    std::cout << "Calculating..." << std::endl;
    for (int i=0; i<101; i++) {
      for (int j=0; j<101; j++) {
        //calculate the average value of 4 adjacent squares, bar corner
        if (j==0) {
          if (i==0) {
            averageVal = average("ES", lastGrid, i, j);
          }
          else if (i==100) {
            averageVal = average("SW", lastGrid, i, j);
          }
          else {
            averageVal = average("ESW", lastGrid, i, j);
          }
        }
        else if (j==100) {
          if (i==0) {
            averageVal = average("NE", lastGrid, i, j);
          }
          else if (i==100) {
            averageVal = average("NW", lastGrid, i, j);
          }
          else {
            averageVal = average("NEW", lastGrid, i, j);
          }
        }
        else if (i==0) {
          averageVal = average("NES", lastGrid, i, j);
        }
        else if (i==100) {
          averageVal = average("NSW", lastGrid, i, j);
        }
        else {
          averageVal = average("NESW", lastGrid, i, j);
        }

        //check if values have converged
        if (std::abs(averageVal-lastGrid[i][j])> 1e-4f) {
          tempChange = true;
        }
        currentGrid[i][j] = averageVal;
      }
    }
  }

  std::cout << "Equilibrium state reached. Centroid temp is " << currentGrid[55][55] << " K" << std::endl;

  return 0;
}

double average(std::string neighboursList, double (*grid)[101], int i, int j) {
  double sum = grid[i][j];
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
