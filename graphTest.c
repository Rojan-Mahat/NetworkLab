#include <stdio.h>
#include <stdlib.h>
#include "network.h"
#include "graph.h"



int main(void) {
    srand(42);  // Reproducible tests
    initializeGraph();

    printf("=== NETWORKLAB GRAPH TEST ===\n");

    // Create 20 networks
    for (int i = 0; i < 20; i++) {
        if (!createNewConnection()) {
            printf("FAIL: Network creation at %d\n", i + 1);
            return 1;
        }
    }

    printf("Networks created: %d\n", getNetworkCount());

    // Add extra links
    int extras = createAllExtraConnections();
    printf("Extra links: %d (expected 5)\n", extras);

    if (extras != 5) {
        printf("FAIL: Extra link target not reached\n");
        return 1;
    }

    printf("\n=== INITIAL GRAPH ===\n");
    printCostsTable();

    // Remove an arbitrary network
    printf("\n=== REMOVE NETWORK 5 ===\n");

    if (!removeNetwork(5)) {
        printf("FAIL: Network removal\n");
        return 1;
    }

    printf("Networks remaining: %d\n", getNetworkCount());

    // Recreate extra connections after topology rebuild.
    // Skip this call if removeNetwork() already rebuilds extras.
    extras = createAllExtraConnections();

    printf("Extra links after rebuild: %d (expected 4)\n", extras);

    printf("\n=== REBUILT GRAPH ===\n");
    printCostsTable();

    // Reinsert a network
    printf("\n=== ADD NEW NETWORK ===\n");

    if (!createNewConnection()) {
        printf("FAIL: Reinsertion\n");
        return 1;
    }

    printf("Final network count: %d\n", getNetworkCount());

    extras = createAllExtraConnections();
    printf("Final extra links: %d (expected 5)\n", extras);

    printf("\n=== FINAL GRAPH ===\n");
    printCostsTable();

    return 0;
}
