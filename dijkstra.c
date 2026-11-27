#include "dijkstra.h"
#include "graph.h"
#include "network.h"
#include "link.h"

#include <math.h>


static double costs[40];
static bool visited[40];
static int previous[40];


bool initializeContainers(int source) {
    if (source < 1 || source > getNetworkCount())//ERROR NO VALID ID
        return false;

    for(int i = 0; i < getNetworkCount(); i++){
        costs[i] = INFINITY;
        visited[i] = false;
        previous[i] = -1;
    }

    costs[source - 1] = 0;
    return true;
}

int findCheapestUnvisited(){
    double cheapest = INFINITY;
    int cheapestIndex = -1;
    for(int i = 0 ; i < getNetworkCount() ; i++){
        if(!visited[i] && getNetwork(i + 1).working && costs[i] < cheapest){
            cheapest = costs[i];
            cheapestIndex = i;
        }
    }
    return cheapestIndex;
}

void updateNotebook(int currentIndex){
    if(!valid(currentIndex + 1))
        return;

    double costTillNow = costs[currentIndex];
    int count = getNetworkCount();

    for(int i = 0 ; i < count ; i++){
        Link currentLink = getLink(currentIndex + 1, i+1);

        if(isEmptyLink(currentLink) || visited[i] || !getNetwork(i + 1).working)
            continue;

        double cost = currentLink.cost;

        if(costTillNow + cost < costs[i]){
            previous[i] = currentIndex;
            costs[i] = costTillNow + cost;
        }
    }
}


PathResult findShortestPath(int sourceID, int destinationID){
    PathResult path;
    path.count = 0;
    for(int i = 0 ; i < 40 ; i++)
        path.paths[i] = -1;

    if(!valid(sourceID) || !valid(destinationID) || !getNetwork(sourceID).working || !getNetwork(destinationID).working)
        return path;
    
    initializeContainers(sourceID);
    while(true){
        int currentIndex = findCheapestUnvisited();
        if (currentIndex == -1)
            break; // No reachable unvisited router remains

        if (currentIndex == destinationID - 1)
            break; 

        updateNotebook(currentIndex);
        visited[currentIndex] = true;
    }

    if (isinf(costs[destinationID - 1]))
        return path;

    path.count = 0;
    int current = destinationID - 1;

    while(current != -1) {
        path.paths[path.count] = current + 1;
        path.count++;
        current = previous[current];
    }

    int left = 0;
    int right = path.count - 1;
    while(left < right){
        int temp = path.paths[left];
        path.paths[left] = path.paths[right];
        path.paths[right] = temp;
        left++;
        right--;
    }
    return path;
}

