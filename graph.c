#include "graph.h"
#include "network.h"

#include <stdio.h>
#include <math.h>

static double costs[40][40];

void initializeGraph(){
  for(int i = 0 ; i < 40 ; i++){
    for(int j = 0 ; j < 40 ; j++){
      if(j == i)
        costs[i][j]= 0;
      else
        costs[i][j]= -1;
    }
  }
}

int searchForConnection(int id){
  if(id < 1 || id > getNetworkCount())
    return -1;

  float minDistance = INFINITY;
  int bestId = -1;

  for(int i = 0 ; i < id - 1 ; i++){
    if(!getHolder()[i].working)
      continue;

    int currentId = getHolder()[i].id;

    float distance = distanceBetween(getHolder()[i], getHolder()[id-1]);

    if(distance < minDistance){
      minDistance = distance;
      bestId = currentId;
    }
  }

  return bestId;
}

void refreshConnection(){
  initializeGraph();
  for(int i = 0 ; i < getNetworkCount() ; i++){
    int bestId = searchForConnection(i + 1);
    if(bestId != -1)
      updateGraph(i, bestId - 1, generateCost(i+1 , bestId));
  }
}

bool createNewConnection(){
  if(generateRandomNetwork(getHolder())){
    if(getNetworkCount() == 1)
      return true;

    int newId = getHolder()[getNetworkCount() - 1].id;
    int bestId = searchForConnection(newId);

    if(bestId != -1 ){
      if(updateGraph(getNetworkCount() - 1, bestId - 1, generateCost(newId , bestId)))
        return true;
    }

    removeNetwork(newId);
    return false;
    //Error All routers are off
  }
  return false;
  //Error failed to create network
}

double generateCost(int id1, int id2){
  (void)id1;
  (void)id2;
  return 999;
}

bool updateGraph(int index1 , int index2 , double cost){
  if(index1 != index2 && index1 < 40 && index2 < 40 && index1 >= 0 && index2 >= 0 && (cost > 0 || cost == -1)){
    costs[index1][index2] = cost;
    costs[index2][index1] = cost;
    return true;
  }
  return false;
}

void printCostsTable(){
  for(int i = 0; i < 40 ; i++){
    for (int j = 0 ; j < 40 ; j++){
      printf("%7.0f", costs[i][j]);
    }
    printf("\n");
  }
}




