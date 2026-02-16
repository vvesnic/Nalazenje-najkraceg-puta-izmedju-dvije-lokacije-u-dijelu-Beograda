#include "pathfinder.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// implementacija reda sa prioritetom
typedef struct {
    Node *node;
    double dist;
} PQNode;

typedef struct {
    PQNode *nodes;
    int size;
    int capacity;
} MinHeap;

MinHeap* createMinHeap(int capacity) {
    MinHeap *h = (MinHeap*) malloc(sizeof(MinHeap));
    h->size = 0;
    h->capacity = capacity;
    h->nodes = (PQNode*) malloc(capacity * sizeof(PQNode));
    return h;
}

void swap(PQNode *a, PQNode *b) {
    PQNode temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapify(MinHeap *h, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < h->size && h->nodes[left].dist < h->nodes[smallest].dist)
        smallest = left;

    if (right < h->size && h->nodes[right].dist < h->nodes[smallest].dist)
        smallest = right;

    if (smallest != idx) {
        swap(&h->nodes[smallest], &h->nodes[idx]);
        minHeapify(h, smallest);
    }
}

void push(MinHeap *h, Node *node, double dist) {
    if (h->size == h->capacity) return; // trebalo bi promijeniti velicinu

    int i = h->size++;
    h->nodes[i].node = node;
    h->nodes[i].dist = dist;

    while (i != 0 && h->nodes[(i - 1) / 2].dist > h->nodes[i].dist) {
        swap(&h->nodes[i], &h->nodes[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

PQNode pop(MinHeap *h) {
    if (h->size <= 0) {
        PQNode empty = {NULL, -1};
        return empty;
    }
    if (h->size == 1) {
        h->size--;
        return h->nodes[0];
    }

    PQNode root = h->nodes[0];
    h->nodes[0] = h->nodes[h->size - 1];
    h->size--;
    minHeapify(h, 0);

    return root;
}

int isEmpty(MinHeap *h) {
    return h->size == 0;
}

// Dijkstrin algoritam
PathResult findShortestPath(Graph *g, long long startNodeId, long long endNodeId) {
    PathResult result;
    result.distance = -1;
    result.pathNodes = NULL;
    result.pathLength = 0;

    Node *startNode = findNode(g, startNodeId);
    Node *endNode = findNode(g, endNodeId);

    if (!startNode || !endNode) {
        printf("Start or end node not found.\n");
        return result;
    }
    
    // Inicijalizuj
    Node *curr = g->nodes;
    while (curr != NULL) {
        curr->dist = DBL_MAX;
        curr->visited = 0;
        curr->parent = NULL;
        curr = curr->nextGlobal;
    }

    
    startNode->dist = 0;
    
    MinHeap *pq = createMinHeap(g->numNodes + 100); // sigurna velicina
    push(pq, startNode, 0);
    
    while (!isEmpty(pq)) {
        PQNode minNode = pop(pq);
        Node *u = minNode.node;
        
        if (u->visited) continue;
        u->visited = 1;
        
        if (u == endNode) break;
        
        Edge *e = u->edges;
        while (e != NULL) {
            Node *v = findNode(g, e->targetNodeId);
            if (v && !v->visited) {
                double newDist = u->dist + e->weight;
                if (newDist < v->dist) { // azuriranje komsije
                    v->dist = newDist;
                    v->parent = u;
                    push(pq, v, newDist);
                }
            }
            e = e->next;
        }
    }
    
    free(pq->nodes);
    free(pq);
    
    if (endNode->dist != DBL_MAX) {
        result.distance = endNode->dist;
        
        // Rekonstruisi putanju
        int count = 0;
        curr = endNode;
        while (curr != NULL) {
            count++;
            curr = curr->parent;
        }
        
        result.pathLength = count;
        result.pathNodes = (long long*) malloc(count * sizeof(long long));
        
        curr = endNode;
        for (int i = count - 1; i >= 0; i--) {
            result.pathNodes[i] = curr->id;
            curr = curr->parent;
        }
    }
    
    return result;
}

void freePathResult(PathResult result) {
    if (result.pathNodes) free(result.pathNodes);
}
