#pragma once
#include<stdbool.h>

struct Network{
    int id;
    float x, y, radius, health;
    bool working;
};


int getNetworkCount();

double distanceBetween(struct Network network1, struct Network network2);
bool generateRandomNetwork(struct Network holder[]);
bool removeNetwork(int id);

struct Network *getHolder(void);

