#define _GNU_SOURCE
#include "graph.h"
#include <stdio.h>
#include <string.h>
#include "../utils/levenstajn.h"

#define HASH_SIZE 10007 // prost broj za velicinu hes tabele

Graph* createGraph(int capacity) {
    Graph *g = (Graph*) malloc(sizeof(Graph));
    g->numNodes = 0;
    g->capacity = capacity;
    g->nodes = NULL;
    
    // Alociraj hes mapu
    g->nodeMap = (Node**) calloc(HASH_SIZE, sizeof(Node*));
    if (!g->nodeMap) {
        fprintf(stderr, "Error: Failed to allocate nodeMap\n");
        free(g);
        return NULL;
    }
    
    return g;
}

// jednostavna hes funkcija za long long ID-eve
unsigned int hash(long long id) {
    if (id < 0) id = -id;
    return (unsigned int)(id % HASH_SIZE);
}

void addNode(Graph *g, long long id, double lat, double lon, const char *name) {
    Node *newNode = (Node*) malloc(sizeof(Node));
    newNode->id = id;
    newNode->lat = lat;
    newNode->lon = lon;
    newNode->name = name ? strdup(name) : NULL;
    newNode->edges = NULL;
    
    // dodaj u povezanu listu svih cvorova (za iteraciju/ciscenje)
    newNode->nextGlobal = g->nodes;
    g->nodes = newNode;
    
    // Dodaj u hes mapu
    unsigned int h = hash(id);
    newNode->next = g->nodeMap[h];
    g->nodeMap[h] = newNode;
    
    g->numNodes++;
}

Node* findNode(Graph *g, long long id) {
    unsigned int h = hash(id);
    Node *curr = g->nodeMap[h];
    while (curr != NULL) {
        if (curr->id == id) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

// pomocna funkcija za pretragu podstringa neosjetljivu na velicinu slova
int containsIgnoreCase(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    
    // Jednostavna implementacija
    int hLen = strlen(haystack);
    int nLen = strlen(needle);
    if (nLen > hLen) return 0;
    
    for (int i = 0; i <= hLen - nLen; i++) {
        int match = 1;
        for (int j = 0; j < nLen; j++) {
            char h = haystack[i+j];
            char n = needle[j];
            
            // pretvori u mala slova
            if (h >= 'A' && h <= 'Z') h += 32;
            if (n >= 'A' && n <= 'Z') n += 32;
            
            if (h != n) {
                match = 0;
                break;
            }
        }
        if (match) return 1;
    }
    return 0;
}

// Petragu podstringa neosjetljivu na velicinu slova
Node** findNodesByName(Graph *g, const char *search, int *count) {
    *count = 0;
    if (!search || strlen(search) == 0) return NULL;
    
    // prvi prolaz: prebroj poklapanja
    int matches = 0;
    Node *curr = g->nodes;
    while (curr != NULL) {
        if (curr->name && containsIgnoreCase(curr->name, search)) {
            matches++;
        }
        curr = curr->nextGlobal;
    }
    
    if (matches == 0) return NULL;
    
    Node **result = (Node**) malloc(matches * sizeof(Node*));
    *count = matches;
    
    // Drugi prolaz: popuni rezultat
    int idx = 0;
    curr = g->nodes;
    while (curr != NULL) {
        if (curr->name && containsIgnoreCase(curr->name, search)) {
            result[idx++] = curr;
        }
        curr = curr->nextGlobal;
    }
    
    return result;
}

Node** findNodesFuzzy(Graph *g, const char *search, int max_dist, int *count) {
    *count = 0;
    if (!search || strlen(search) == 0) return NULL;
    
    // Privremeni niz za cuvanje pogodaka
    Node **temp_results = (Node**) malloc(1000 * sizeof(Node*));
    int matches = 0;
    
    Node *curr = g->nodes;
    while (curr != NULL) {
        if (curr->name) {
            int dist = levenshtein_distance(curr->name, search);
            if (dist <= max_dist) {
                if (matches < 1000) {
                    temp_results[matches++] = curr;
                }
            }
        }
        curr = curr->nextGlobal;
    }
    
    if (matches == 0) {
        free(temp_results);
        return NULL;
    }
    
    *count = matches;
    Node **result = (Node**) malloc(matches * sizeof(Node*));
    for (int i = 0; i < matches; i++) {
        result[i] = temp_results[i];
    }
    
    free(temp_results);
    return result;
}

Node* getNearestNode(Graph *g, double lat, double lon) {
    Node *nearest = NULL;
    double minDist = 1e9; // velika pocetna udaljenost
    
    Node *curr = g->nodes;
    while (curr != NULL) {
        // Samo razmotri cvorove koji imaju ivice (dio su putne mreze)
        if (curr->edges != NULL) {
            // jednostavna Euklidska aproksimacija je dovoljna za lokalno pronalazenje najblizeg cvora,
            // ali mozemo koristiti Haversine ako ukljucimo geometry.h. 
            // Posto smo u model/graph.c a geometry je u utils, koristimo jednostavni kvadratni Euklid 
            // na lat/lon radi brzine, jer nam samo treba minimum.
            // napomena: 1 stepen sirine je ~111km, 1 stepen duzine je ~80km na ovoj sirini.
            // Mozemo ih tretirati priblizno jednako za nalazenje *najblize* tacke u malom radiusu.
            
            double dLat = curr->lat - lat;
            double dLon = curr->lon - lon;
            double distSq = dLat*dLat + dLon*dLon;
            
            if (distSq < minDist) {
                minDist = distSq;
                nearest = curr;
            }
        }
        curr = curr->nextGlobal;
    }
    return nearest;
}

void addEdge(Graph *g, long long srcId, long long destId, double weight, const char *name) {
    Node *srcNode = findNode(g, srcId);
    // ne moramo striktno pronaci destNode da bismo dodali ivicu u listu srcNode-a,
    // ali je dobra praksa osigurati da postoji.
    // Za ovu implementaciju, pretpostavljamo da su cvorovi dodati prije ivica.
    
    if (srcNode == NULL) return;

    Edge *newEdge = (Edge*) malloc(sizeof(Edge));
    newEdge->targetNodeId = destId;
    newEdge->weight = weight;
    newEdge->name = strdup(name ? name : "");
    newEdge->next = srcNode->edges;
    srcNode->edges = newEdge;
}

void freeGraph(Graph *g) {
    // Iteriraj kroz hes mapu da oslobodis cvorove
    for (int i = 0; i < HASH_SIZE; i++) {
        Node *curr = g->nodeMap[i];
        while (curr != NULL) {
            Node *temp = curr;
            curr = curr->next;
            
            // oslobodi ivice
            Edge *e = temp->edges;
            while (e != NULL) {
                Edge *eTemp = e;
                e = e->next;
                free(eTemp->name);
                free(eTemp);
            }
            if (temp->name) free(temp->name);
            free(temp);
        }
    }
    free(g->nodeMap);
    free(g);
}
