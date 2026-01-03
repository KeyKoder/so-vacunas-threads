#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_CENTROS_VACUNACION 5
#define NUM_FABRICAS 3
#define NUM_TANDAS 10

#define MAX_LINE_LENGTH 1024

char* defaultInfileName = "entrada_vacunacion.txt";
char* defaultOutfileName = "salida_vacunacion.txt";

struct Config {
    int totalHabitantes;
    int initialVacunas;
    
    int minVacunasFabricadas;
    int maxVacunasFabricadas;
    
    int minTiempoFabricacion;
    int maxTiempoFabricacion;
    int maxTiempoReparto;
    
    int maxTiempoReaccion;
    int maxTiempoDespl;
};

typedef struct Config config_t;

config_t config;

// returns whether it could read the config file or not
int readConfig(char* filename);
void printConfig();


int randInt(int min, int max);

// TODO: Create structs(?) for vac. centers and suppliers
// TODO: Implement producer-consumer logic

int main(int argc, char** argv) {
    char* infileName = defaultInfileName;
    char* outfileName = defaultOutfileName;
    
    if(argc > 3) {
        printf("Uso: %s [nombre_fichero_entrada] [nombre_fichero_salida]\n", argv[0]);
        exit(1);
    }

    if(argc >= 2) {
        infileName = argv[1];
    }

    if(argc == 3) {
        outfileName = argv[2];
    }

    // printf("infile: %s\noutfile: %s\n\n", infileName, outfileName);


    if(readConfig(infileName) != 0) {
        fprintf(stderr, "Error reading config file: %s\n", infileName);
        exit(1);
    }
    printConfig();

    exit(0);
}

int readConfig(char* filename) {
    FILE* file;
    char str[MAX_LINE_LENGTH];
    int fieldIdx = 0;
    
    if (access(filename, F_OK) == 0) {
        file = fopen(filename, "r");
    }else {
        return 1;
    }
    config = (config_t){};
    
    while(fgets(str, MAX_LINE_LENGTH, file) != NULL) {
        // Check if line fails to parse, using the fact that atoi atoi returns 0 if it cant parse str into an int and checking if the str contains 0
        if(atoi(str) == 0 && str[0] != '0') {
            printf("Cannot parse int at line %d \"%.*s\"\n", fieldIdx+1, (int)strlen(str)-1, str);
            fclose(file);
            return 1;
        }

        // Index the corresponding field of the struct dynamically by doing pointer arithmetic
        *((int*)&config+fieldIdx) = atoi(str);

        fieldIdx++;
    }

    fclose(file);
    return 0;
}

void printConfig() {
    printf("Habitantes: %d\nCentros de vacunación: %d\nFábricas: %d\nVacunados por tanda: %d\nVacunas iniciales en cada centro: %d\nVacunas totales por fábrica: %d\nMínimo número de vacunas fabricadas en cada tanda: %d\nMáximo número de vacunas fabricadas en cada tanda: %d\nTiempo mínimo de fabricación de una tanda de vacunas: %d\nTiempo máximo de fabricación de una tanda de vacunas: %d\nTiempo máximo de reparto de vacunas a los centros: %d\nTiempo máximo que un habitante tarda en ver que está citado para vacunarse: %d\nTiempo máximo de desplazamiento del habitante al centro de vacunación: %d\n", 
        config.totalHabitantes, 
        NUM_CENTROS_VACUNACION, 
        NUM_FABRICAS, 
        config.totalHabitantes/NUM_TANDAS, 
        config.initialVacunas, 
        config.totalHabitantes/NUM_FABRICAS, 
        config.minVacunasFabricadas, 
        config.maxVacunasFabricadas, 
        config.minTiempoFabricacion, 
        config.maxTiempoFabricacion, 
        config.maxTiempoReparto, 
        config.maxTiempoReaccion, 
        config.maxTiempoDespl);
}

int randInt(int min, int max) {
    return rand() % (max - min + 1) + min;
}
