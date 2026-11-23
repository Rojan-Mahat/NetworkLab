#include "graph.h"
#include "network.h"
#include "link.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static Link links[40][40];
static int extraLinks = 0;

void initializeGraph(){
  extraLinks = 0;
  for(int i = 0 ; i < 40 ; i++){
    for(int j = 0 ; j < 40 ; j++){
      if(j == i)
        links[i][j]= createSelfLink();
      else
        links[i][j]= createEmptyLink();
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
      updateGraph(i+1, bestId);
  }
}

bool createNewConnection(){
  if(generateRandomNetwork(getHolder())){
    if(getNetworkCount() == 1)
      return true;

    int newId = getHolder()[getNetworkCount() - 1].id;
    int bestId = searchForConnection(newId);

    if(bestId != -1 ){
      if(updateGraph(newId, bestId))
        return true;
    }

    removeNetwork(newId);
    return false;
    //Error All routers are off
  }
  return false;
  //Error failed to create network
}

bool updateGraph(int id1 ,int id2){
  if(id1 < 1 || id2 < 1 || id1 > getNetworkCount() || id2 > getNetworkCount() || id1 == id2)
      return false;

  Link link = createLink(id1 ,id2);
  links[id1-1][id2-1] = link;
  links[id2-1][id1-1] = link;
  return true;
}

void printCostsTable(){
  for(int i = 0; i < getNetworkCount() ; i++){
    for (int j = 0 ; j < getNetworkCount() ; j++){
        printf("%7.0f", links[i][j].cost);
    }
    printf("\n");
  }
}

bool hasConnection(int id1 , int id2){
  if(id1 > 0 && id2 > 0 && id1 <= getNetworkCount() && id2 <= getNetworkCount()){
      return links[id1 - 1][id2 -1].cost > 0;
  }
  return false;
}

int createAllExtraConnections(){
  if(getNetworkCount() < 2)
    return 0;
  int tries = 0;
  while(extraLinks < getNetworkCount() / 4 && tries < 5000){
    tries++;
    addConnection();
  }
  return extraLinks;
}


bool addConnection(){
  if(getNetworkCount() < 2)
    return false;

  int id1 = rand() % getNetworkCount() + 1;
  int id2 = rand() % getNetworkCount() + 1;
  if(id1 != id2 && !hasConnection(id1 , id2)){
    if(updateGraph(id1 , id2)){
      extraLinks++;
      return true;
    }
  }
  return false;
}

Link getLink(int id1, int id2){
  int count = getNetworkCount();
  if(id1 > 0 && id1 <= count && id2 > 0 && id2 <= count)
    return links[id1-1][id2-1];
  return createEmptyLink();
}


