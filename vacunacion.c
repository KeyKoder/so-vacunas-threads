#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_CENTROS_VACUNACION 5
#define NUM_FABRICAS 3
#define NUM_TANDAS 10

#define MAX_LINE_LENGTH 10

char* defaultInfileName = "entrada_vacunacion.txt";
char* defaultOutfileName = "salida_vacunacion.txt";

// TODO: Create global config struct to keep the initial parameters so all the threads can access them easily

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

// TODO: Read config from file
int readConfig(char*);
void printConfig();


// TODO: Create helper function for rand() with min and max for convenience
// TODO: Create structs(?) for vac. centers and suppliers
// TODO: Implement producer-consumer logic

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


    if(!readConfig(infileName)) {
        fprintf(stderr, "Error reading config file: %s\n", infileName);
        exit(1);
    }
    printConfig();

    exit(0);
}


// returns whether it could read the config file or not
int readConfig(char* filename) {
    FILE* file;
    if (access(filename, F_OK) == 0) {
        file = fopen(filename, "r");
    }else {
        return 0;
    }
    config = (config_t){};

    char str[MAX_LINE_LENGTH];
    int fieldIdx = 0;
    
    while(fgets(str, MAX_LINE_LENGTH, file) != NULL) {
        // Index the corresponding field of the struct dynamically by doing pointer arithmetic
        *((int*)&config+fieldIdx*sizeof(int)) = atoi(str);

        fieldIdx++;
    }

    fclose(file);
    return 1;
}

void printConfig() {
    printf("Habitantes: %d\nCentros de vacunación: %d\nFábricas: %d\nVacunados por tanda: %d\nVacunas iniciales en cada centro: %d\nVacunas totales por fábrica: %d\nMínimo número de vacunas fabricadas en cada tanda: %d\nMáximo número de vacunas fabricadas en cada tanda: %d\nTiempo mínimo de fabricación de una tanda de vacunas: %d\nTiempo máximo de fabricación de una tanda de vacunas: %d\nTiempo máximo de reparto de vacunas a los centros: %d\nTiempo máximo que un habitante tarda en ver que está citado para vacunarse: %d\nTiempo máximo de desplazamiento del habitante al centro de vacunación: %d\n", config.totalHabitantes, NUM_CENTROS_VACUNACION, NUM_FABRICAS, config.totalHabitantes/NUM_TANDAS, config.initialVacunas, config.totalHabitantes/NUM_FABRICAS, config.minVacunasFabricadas, config.maxVacunasFabricadas, config.minTiempoFabricacion, config.maxTiempoFabricacion, config.maxTiempoReparto, config.maxTiempoReaccion, config.maxTiempoDespl);
}
