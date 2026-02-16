#include "parser.h"
#include "../utils/geometry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// pomocna funkcija za izvlacenje vrijednosti atributa iz linije
// Vraca novoalocirani string ili NULL ako nije pronadjen
char* getAttr(const char *line, const char *attrName) {
    char search[64];
    sprintf(search, "%s=\"", attrName);
    
    char *start = strstr(line, search);
    if (!start) {
        return NULL;
    }
    
    start += strlen(search);
    char *end = strchr(start, '"');
    if (!end) return NULL;
    
    int len = end - start;
    char *val = (char*) malloc(len + 1);
    strncpy(val, start, len);
    val[len] = '\0';
    
    return val;
}

int parseMap(const char *filename, Graph *g) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Greska: nije moguce otvoriti fajl \"%s\"\n", filename);
        return -1;
    }

    char line[1024];
    int inWay = 0;
    int inNode = 0;
    long currentWayId = -1;
    long currentNodeId = -1;
    double currentLat = 0, currentLon = 0;
    char *currentNodeName = NULL;
    
    long *nodeRefs = (long*) malloc(50000 * sizeof(long));
    if (!nodeRefs) {
        fprintf(stderr, "Greska: nema dovoljno memorije za nodeRefs\n");
        fclose(fp);
        return -1;
    }
    printf("nodeRefs alociran.\n");
    fflush(stdout);

    int refCount = 0;
    int isHighway = 0;
    char *wayName = NULL;

    printf("Ucitavanje mape (custom parser)...\n");
    fflush(stdout);
    
    long lineCount = 0;
    while (fgets(line, sizeof(line), fp)) {
        lineCount++;
        // pocetak cvora
        if (strstr(line, "<node")) {
            char *idStr = getAttr(line, "id");
            char *latStr = getAttr(line, "lat");
            char *lonStr = getAttr(line, "lon");
            
            if (idStr && latStr && lonStr) {
                currentNodeId = atol(idStr);
                currentLat = atof(latStr);
                currentLon = atof(lonStr);
                inNode = 1;
                if (currentNodeName) { free(currentNodeName); currentNodeName = NULL; }
                
                // provjera da li je samozatvarajuci
                if (strstr(line, "/>")) {
                    addNode(g, currentNodeId, currentLat, currentLon, NULL);
                    inNode = 0;
                }
            }
            
            if (idStr) free(idStr);
            if (latStr) free(latStr);
            if (lonStr) free(lonStr);
        }
        // kraj cvora
        else if (inNode && strstr(line, "</node>")) {
            addNode(g, currentNodeId, currentLat, currentLon, currentNodeName);
            if (currentNodeName) { free(currentNodeName); currentNodeName = NULL; }
            inNode = 0;
        }
        // Pocetak puta (way)
        else if (strstr(line, "<way")) {
            inWay = 1;
            refCount = 0;
            isHighway = 0;
            if (wayName) { free(wayName); wayName = NULL; }
            
            char *idStr = getAttr(line, "id");
            if (idStr) {
                currentWayId = atol(idStr);
                free(idStr);
            }
        }
        // kraj puta (way)
        else if (inWay && strstr(line, "</way>")) {
            if (isHighway && refCount > 1) {
                for (int j = 0; j < refCount - 1; j++) {
                    long u = nodeRefs[j];
                    long v = nodeRefs[j+1];
                    
                    Node *nodeU = findNode(g, u);
                    Node *nodeV = findNode(g, v);
                    
                    if (nodeU && nodeV) {
                        double dist = calculateDistance(nodeU->lat, nodeU->lon, nodeV->lat, nodeV->lon);
                        addEdge(g, u, v, dist, wayName);
                        addEdge(g, v, u, dist, wayName); // Neusmjereno
                    }
                }
            }
            inWay = 0;
            if (wayName) { free(wayName); wayName = NULL; }
        }
        // referenca na cvor u putu
        else if (inWay && strstr(line, "<nd")) {
            char *refStr = getAttr(line, "ref");
            if (refStr) {
                if (refCount < 50000) {
                    nodeRefs[refCount++] = atol(refStr);
                }
                free(refStr);
            }
        }
        // tagovi (i za cvorove i za puteve)
        else if ((inWay || inNode) && strstr(line, "<tag")) {
            char *k = getAttr(line, "k");
            char *v = getAttr(line, "v");
            
            if (k && v) {
                if (inWay) {
                    if (strcmp(k, "highway") == 0) {
                        isHighway = 1;
                    }
                    if (strcmp(k, "name") == 0 || strcmp(k, "name:sr-Latn") == 0 || strcmp(k, "int_name") == 0) {
                        if (wayName == NULL) {
                            wayName = strdup(v);
                        } 
                        else {
                            // dodaj ako vec nije prisutno (jednostavna provjera)
                            if (!strstr(wayName, v)) {
                                size_t newLen = strlen(wayName) + strlen(v) + 4;
                                char *newName = (char*) malloc(newLen);
                                sprintf(newName, "%s / %s", wayName, v);
                                free(wayName);
                                wayName = newName;
                            }
                        }
                    }
                } 
                else if (inNode) {
                    if (strcmp(k, "name") == 0 || strcmp(k, "name:sr-Latn") == 0 || strcmp(k, "int_name") == 0) {
                        if (currentNodeName == NULL) {
                            currentNodeName = strdup(v);
                        } 
                        else {
                            // Dodaj ako vec nije prisutno
                            if (!strstr(currentNodeName, v)) {
                                size_t newLen = strlen(currentNodeName) + strlen(v) + 4;
                                char *newName = (char*) malloc(newLen);
                                sprintf(newName, "%s / %s", currentNodeName, v);
                                free(currentNodeName);
                                currentNodeName = newName;
                            }
                        }
                    }
                }
            }
            
            if (k) free(k);
            if (v) free(v);
        }
    }

    free(nodeRefs);
    fclose(fp);
    return 0;
}
