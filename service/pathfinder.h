#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "../model/graph.h"

typedef struct PathResult {
    double distance;
    long long *pathNodes; // niz ID-eva cvorova u putanji
    int pathLength;
} PathResult;

PathResult findShortestPath(Graph *g, long long startNodeId, long long endNodeId);

void freePathResult(PathResult result);

#endif
