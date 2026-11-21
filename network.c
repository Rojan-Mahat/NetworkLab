#include "network.h"
#include "graph.h"

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<stdbool.h>

static int networkCount = 0;

int getNetworkCount(){
    return networkCount;
}

double distanceBetween(struct Network network1, struct Network network2){
    return sqrt(pow(network2.x - network1.x, 2) + pow(network2.y - network1.y, 2)); 
}

bool generateRandomNetwork(struct Network holder[]){
    if(networkCount >= 40)
        return false;
    int tries = 0;
    float x,y,r;
    while(true){
        bool found = true;
        x = (rand() % (9950 - 50 + 1) + 50);
        y = (rand() % (9950 - 50 + 1) + 50);
        r = (rand() % (400 - 50 + 1) + 50);
        tries++;

        if(tries > 10000){
            fprintf(stderr, "No valid location to create network");
            return false;
        }

        for(int i = 0; i < networkCount ; i++){
            float dx = holder[i].x - x;
            float dy = holder[i].y - y;

            float distanceSquared = dx * dx + dy * dy;
            float radiusSumSquared = (r + holder[i].radius + 400) * (r + holder[i].radius + 400);

           if(distanceSquared < radiusSumSquared){
                found = false;
                break;
            }
        }

        if(found)
            break;
    }
    struct Network n1;
    n1.id = networkCount + 1;
    n1.x = x;
    n1.y = y;
    n1.radius = r;
    n1.health = 100;
    n1.working = true;
    holder[networkCount] = n1;
    networkCount++;
    return true;
}


bool removeNetwork(int id){
    if(id < 1 || id > networkCount)
        return false;
    //error id out of range 

    for(int i = 0 ; i < networkCount ; i++){
        if(getHolder()[i].id == id){

            for(int j = i ; j < networkCount - 1 ; j++)
                getHolder()[j] = getHolder()[j+1];

            networkCount--;
            break;
        }
    }

    for(int a = 0 ; a < networkCount ; a++)
      getHolder()[a].id = a + 1;
    
    refreshConnection();
    return true;
}
