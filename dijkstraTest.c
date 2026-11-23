
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "network.h"
#include "graph.h"
#include "dijkstra.h"

void printPath(int source, int destination) {
    PathResult result = findShortestPath(source, destination);

    printf("\nShortest path from %d to %d:\n", source, destination);

    if (result.count == 0) {
        printf("No path found.\n");
        return;
    }

    double totalCost = 0;

    for (int i = 0; i < result.count; i++) {
        printf("%d", result.paths[i]);

        if (i < result.count - 1) {
            int from = result.paths[i];
            int to = result.paths[i + 1];

            Link link = getLink(from, to);

            if (isEmptyLink(link)) {
                printf("\nFAIL: Route contains a missing link!\n");
                return;
            }
            totalCost += link.cost;
            printf(" -> ");
        }
    }

    printf("\nRouters: %d\n", result.count);
    printf("Hops: %d\n", result.count - 1);
    printf("Total cost: %.4f\n", totalCost);
}

int main(void) {
    srand(42);  
    initializeGraph();

    printf("=== NETWORKLAB GRAPH TEST ===\n");
    for (int i = 0; i < 20; i++) {
        if (!createNewConnection()) {
            printf("FAIL: Network creation at %d\n", i + 1);
            return 1;
        }
    }

    printf("Networks created: %d\n", getNetworkCount());

    int extras = createAllExtraConnections();
    printf("Extra links: %d (expected 5)\n", extras);

    if (extras != 5) {
        printf("FAIL: Extra link target not reached\n");
        return 1;
    }

    printf("=== DIJKSTRA TEST ===\n");

    printPath(1, 20);  // Normal route
    printPath(5, 15);  // Different source
    printPath(1, 1);   // Source equals destination
    printPath(0, 5);   // Invalid source
    printPath(1, 40);  // Invalid destination when count < 40

    return 0;
}
