#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

int main() {
    printf("Libxml2 version: %s\n", LIBXML_DOTTED_VERSION);
    return 0;
}
