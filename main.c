#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model/graph.h"
#include "service/parser.h"
#include "service/pathfinder.h"
#include "utils/geometry.h"

// pomocna funkcija za dobijanje ID-a cvora od korisnika (ID ili ime)
long long getNodeInput(Graph *g, const char *prompt) {
    char input[256];
    while (1) {
        printf("%s (unesite ID ili Ime): ", prompt);
        if (fgets(input, sizeof(input), stdin) == NULL) return -1;
        input[strcspn(input, "\n")] = 0; // Ukloni novi red
        
        // provjeri da li je unos broj
        char *endptr;
        long long id = strtoll(input, &endptr, 10);
        if (*endptr == '\0' && strlen(input) > 0) {
            // To je broj, potvrdi da postoji
            if (findNode(g, id)) {
                return id;
            } 
            else {
                printf("Cvor sa ID-em %lld nije pronadjen.\n", id);
            }
        } 
        else {
            // to je string, pretrazi po imenu
            int count = 0;
            
            // 1. POKUSAJ: Obicna pretraga (podstring, neosjetljiva na slova)
            Node **results = findNodesByName(g, input, &count);
            
            // optimizacija, dodat (levenstajnov algoritam):
            // Ako obicna pretraga nije nasla nista, pokusaj Fuzzy (Levenstajn)
            if (count == 0) {
                printf("Nema tacnog poklapanja za '%s'. Trazim priblizne lokacije...\n", input);
                results = findNodesFuzzy(g, input, 4, &count);
            }

            if (count == 0) {
                printf("Nisu pronadjeni cvorovi cak ni sa pribliznim imenom '%s'.\n", input);
            } 
            else {
                if (count == 1) {
                    printf("Pronadjeno: %s (ID: %lld)\n", results[0]->name, results[0]->id);
                } else {
                    printf("Pronadjeno %d rezultata:\n", count);
                }
                
                int limit = count > 10 ? 10 : count;
                for (int i = 0; i < limit; i++) {
                    printf("%d. %s (ID: %lld)\n", i + 1, results[i]->name, results[i]->id);
                }
                if (count > 10) printf("... i jos %d.\n", count - 10);
                
                printf("Izaberite broj (1-%d) ili 0 za odustajanje/ponovni unos: ", limit);
                if (fgets(input, sizeof(input), stdin)) {
                    int choice = atoi(input);
                    if (choice >= 1 && choice <= limit) {
                        long long resultId = results[choice - 1]->id;
                        free(results);
                        return resultId;
                    }
                }
                free(results);
            }
        }
    }
}

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // Postavi konzolu na UTF-8
#endif
    setbuf(stdout, NULL);
    if (argc < 2) {
        printf("Upotreba: %s <putanja_do_xml_fajla>\n", argv[0]);
        return 1;
    }

    Graph *g = createGraph(100000); // pocetni kapacitet
    if (parseMap(argv[1], g) != 0) {
        printf("Neuspesno ucitavanje mape.\n");
        fflush(stdout);
        freeGraph(g);
        return 1;
    }
    
    printf("Graf ucitan. Cvorova: %d\n", g->numNodes);
    fflush(stdout);

    while (1) {
        printf("\n--- Pronadji Najkraci Put ---\n");
        long long startId = getNodeInput(g, "Pocetna Lokacija");
        if (startId == -1) break;
        
        long long endId = getNodeInput(g, "Krajnja Lokacija");
        if (endId == -1) break;

        // Provjeri da li je pocetni cvor izolovan
        Node *startNode = findNode(g, startId);
        if (startNode && startNode->edges == NULL) {
            printf("\nCvor %lld (%s) je izolovan. Povezivanje sa najblizim putem...\n", startId, startNode->name ? startNode->name : "Nepoznato");
            Node *nearest = getNearestNode(g, startNode->lat, startNode->lon);
            if (nearest) {
                printf("Povezano sa cvorom %lld (%.2f metara udaljeno)\n", nearest->id, calculateDistance(startNode->lat, startNode->lon, nearest->lat, nearest->lon));
                startId = nearest->id;
            } 
            else {
                printf("Nije moguce pronaci obliznji putni cvor.\n");
                continue;
            }
        }

        // provjeri da li je krajnji cvor izolovan
        Node *endNode = findNode(g, endId);
        if (endNode && endNode->edges == NULL) {
            printf("\nCvor %lld (%s) je izolovan. Povezivanje sa najblizim putem...\n", endId, endNode->name ? endNode->name : "Nepoznato");
            Node *nearest = getNearestNode(g, endNode->lat, endNode->lon);
            if (nearest) {
                printf("Povezano sa cvorom %lld (%.2f metara udaljeno)\n", nearest->id, calculateDistance(endNode->lat, endNode->lon, nearest->lat, nearest->lon));
                endId = nearest->id;
            }
            else {
                printf("Nije moguce pronaci obliznji putni cvor.\n");
                continue;
            }
        }

        PathResult result = findShortestPath(g, startId, endId);

        if (result.distance == -1) {
            printf("\nNije pronadjen put izmedju %lld i %lld.\n", startId, endId);
        } 
        else {
            printf("\nDuzina najkraceg puta: %.2f metara\n", result.distance);
            printf("Putanja: ");
            for (int i = 0; i < result.pathLength; i++) {
                Node *n = findNode(g, result.pathNodes[i]);
                if (n && n->name) {
                    printf("%s", n->name);
                } 
                else {
                    // Ako cvor nema ime, pokusavamo pronaci ime ulice koja vodi do njega
                    char *edgeName = NULL;
                    if (i > 0) {
                        Node *prev = findNode(g, result.pathNodes[i-1]);
                        if (prev) {
                            Edge *e = prev->edges;
                            while (e) {
                                if (e->targetNodeId == result.pathNodes[i]) {
                                    if (e->name && strlen(e->name) > 0) {
                                        edgeName = e->name;
                                    }
                                    break;
                                }
                                e = e->next;
                            }
                        }
                    }
                    
                    if (edgeName) {
                        printf("%s", edgeName);
                    } 
                    else {
                        printf("%lld", result.pathNodes[i]);
                    }
                }
                
                if (i < result.pathLength - 1) printf(" -> ");
            }
            printf("\n");
            freePathResult(result);
        }
        
        printf("\nPronadji drugi put? (d/n): ");
        char buf[10];
        if (fgets(buf, sizeof(buf), stdin) && (buf[0] == 'n' || buf[0] == 'N')) {
            break;
        }
    }

    freeGraph(g);
    return 0;
}
