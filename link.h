#pragma once
#include "network.h"

#include<stdbool.h>

typedef enum{
  SILVER, 
  GOLD,  
  PLATINUM,
  BANDWIDTH_NONE
} BandwidthTier;

typedef enum{
  LOW, 
  MEDIUM,
  HIGH,
  EXTREME,
  LATENCY_NONE
} LatencyTier;

typedef enum {
  COPPER,
  FIBER,
  WIRELESS,
  SATELLITE,
  MEDIUM_NONE
} LinkMedium;

typedef struct Link{
  LinkMedium medium;
  BandwidthTier bandwidth;
  LatencyTier latency;
  double length;
  double cost;
} Link;

struct Link createLink(int id1 , int id2);
struct Link createEmptyLink();
struct Link createSelfLink();
bool isEmptyLink(struct Link link);



