#include "network.h"
#include "graph.h"


#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include <time.h>
#include<stdbool.h>

struct Network netHolder [40];

struct Network* getHolder(){
    return netHolder;
}


int main(){
srand(time(NULL));
    initializeGraph();

    // Create 20 networks
    for (int i = 0; i < 20; i++) {
        if (!createNewConnection()) {
            printf("FAILED to create network %d\n", i + 1);
            return 1;
        }
    }

    printf("\n=== INITIAL NETWORKS ===\n");
    printf("Network count: %d\n", getNetworkCount());
    printCostsTable();

    // Remove a middle network
    printf("\n=== REMOVING NETWORK 5 ===\n");
    printf("Removal: %s\n",
           removeNetwork(5) ? "SUCCESS" : "FAILED");

    printf("Network count: %d\n", getNetworkCount());

    // Verify IDs after shifting
    for (int i = 0; i < getNetworkCount(); i++) {
        printf("Index %2d -> Network ID %2d\n",
               i, getHolder()[i].id);
    }

    printf("\n=== REBUILT GRAPH ===\n");
    printCostsTable();

    // Add another network
    printf("\n=== ADDING NETWORK AGAIN ===\n");
    printf("Creation: %s\n",
           createNewConnection() ? "SUCCESS" : "FAILED");

    printf("Final network count: %d\n", getNetworkCount());
    printCostsTable();
    return 0;
}
