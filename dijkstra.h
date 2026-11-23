#pragma once

#include <stdbool.h>

typedef struct {
        int paths[40];
        int count;
    } PathResult;

bool initializeContainers(int source);
int findCheapestUnvisited();
void updateNotebook(int currentIndex);
PathResult findShortestPath(int sourceID, int destinationID);