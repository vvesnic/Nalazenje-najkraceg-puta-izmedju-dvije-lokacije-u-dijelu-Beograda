#ifndef GRAPH_H
#define GRAPH_H

#include <stdlib.h>

// Struktura cvora koja predstavlja lokaciju na mapi
typedef struct Node {
    long long id;
    double lat;
    double lon;
    char *name; // ime lokacije
    struct Edge *edges; // glava liste ivica
    struct Node *next;  // za hes mapu
    struct Node *nextGlobal; // Za listu svih cvorova
    
    // Za Dijkstru
    double dist;
    int visited;
    struct Node *parent;
} Node;

// struktura ivice koja predstavlja segment ulice
typedef struct Edge {
    long long targetNodeId;
    double weight; // Udaljenost u metrima
    char *name;    // Ime ulice
    struct Edge *next;
} Edge;

// struktura grafa
typedef struct Graph {
    int numNodes;
    Node **nodeMap; // Hes mapa
    int capacity;
    Node *nodes; // Povezana lista svih cvorova za iteraciju
} Graph;

Graph* createGraph(int capacity);

void addNode(Graph *g, long long id, double lat, double lon, const char *name);

void addEdge(Graph *g, long long srcId, long long destId, double weight, const char *name);

Node* findNode(Graph *g, long long id); 

Node** findNodesByName(Graph *g, const char *search, int *count);

// Pronalazi cvorove cije je ime slicno trazenom (Levenstajnova udaljenost)
Node** findNodesFuzzy(Graph *g, const char *search, int max_dist, int *count);

Node* getNearestNode(Graph *g, double lat, double lon);

Node* getNearestNode(Graph *g, double lat, double lon);

void freeGraph(Graph *g);

#endif
