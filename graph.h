#pragma once
#include"link.h"

#include <stdbool.h>

void initializeGraph();

bool createNewConnection();
int searchForConnection(int id);
void refreshConnection();
bool updateGraph(int id1 , int id2);

void printCostsTable();

double generateCost(int id1 , int id2);
bool hasConnection(int id1, int id2);
bool addConnection();
int createAllExtraConnections();
Link getLink(int id1, int id2);
