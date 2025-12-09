#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

char* defaultInfileName = "entrada_vacunacion.txt";
char* defaultOutfileName = "salida_vacunacion.txt";

int main(int argc, char** argv) {
    if(argc > 3) {
        printf("Uso: %s [nombre_fichero_entrada] [nombre_fichero_salida]\n", argv[0]);
        exit(1);
    }

    char* infileName = defaultInfileName;
    char* outfileName = defaultOutfileName;

    if(argc >= 2) {
        infileName = argv[1];
    }

    if(argc == 3) {
        outfileName = argv[2];
    }

    printf("infile: %s\noutfile: %s\n", infileName, outfileName);

    exit(0);
}
