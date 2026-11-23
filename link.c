#include "link.h"
#include "network.h"
#include <stdlib.h>
#include <math.h>

struct Link createLink(int id1 , int id2){
  if(id1 < 1 || id2 < 1 || id1 > getNetworkCount() || id2 > getNetworkCount())
    return createEmptyLink();
  //error id out of range

  struct Network network1 = getHolder()[id1-1];
  struct Network network2 = getHolder()[id2 - 1];

  struct Link link;
  int bandwidthPenalty, latencyPenalty;

  link.length = distanceBetween(network1 , network2);
  double lengthPenalty = link.length * 100  / 14000.7;

  link.medium = rand() % 4;
  if(link.medium == COPPER){
    link.latency = MEDIUM;
    latencyPenalty = 15;

    link.bandwidth = GOLD;
    bandwidthPenalty = 30;
  }

  else if(link.medium == FIBER){
    link.latency = LOW;
    latencyPenalty = 5;

    link.bandwidth = PLATINUM;
    bandwidthPenalty = 10;
  }

  else if(link.medium == WIRELESS){
    link.latency = HIGH; 
    latencyPenalty = 30;

    link.bandwidth = SILVER;
    bandwidthPenalty = 50;
  }

  else{
    link.latency = EXTREME;
    latencyPenalty = 70;

    link.bandwidth = GOLD;
      bandwidthPenalty = 30;
  }

  double latency = fmin(100.0, latencyPenalty + 0.3 * lengthPenalty);
  link.cost = 0.2 * lengthPenalty + 0.4 * bandwidthPenalty + 0.4 * latency;
  return link;
}



struct Link createEmptyLink(){
  struct Link link;
  link.length = -1;
  link.cost = -1;
  link.bandwidth = BANDWIDTH_NONE;
  link.latency = LATENCY_NONE;
  link.medium = MEDIUM_NONE;
  return link;
}


struct Link createSelfLink(){
  struct Link link;
  link.length = 0;
  link.cost = 0;
  link.bandwidth = BANDWIDTH_NONE;
  link.latency = LATENCY_NONE;
  link.medium = MEDIUM_NONE;
  return link;
}

bool isEmptyLink(struct Link link){
  return link.cost <= 0;
}






