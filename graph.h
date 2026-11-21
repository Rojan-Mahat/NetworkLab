#pragma once
#include <stdbool.h>

void initializeGraph();

bool createNewConnection();
int searchForConnection(int id);
void refreshConnection();
bool updateGraph(int index1 , int index2 , double cost);

void printCostsTable();

double generateCost(int id1 , int id2);
