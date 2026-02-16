#include "levenstajn.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static int min3(int a, int b, int c) {
    int m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    return m;
}

int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    
    int **matrix = (int**) malloc((len1 + 1) * sizeof(int*));
    for (int i = 0; i <= len1; i++) {
        matrix[i] = (int*) malloc((len2 + 1) * sizeof(int));
    }
    
    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;
    
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            
            int cost = (tolower(s1[i - 1]) == tolower(s2[j - 1])) ? 0 : 1;
            
            matrix[i][j] = min3(
                matrix[i - 1][j] + 1,      // brisanje
                matrix[i][j - 1] + 1,      // ubacivanje
                matrix[i - 1][j - 1] + cost // zamjena
            );
        }
    }
    
    int result = matrix[len1][len2];
    
    // Oslobađanje memorije
    for (int i = 0; i <= len1; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return result;
}
