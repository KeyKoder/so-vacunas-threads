#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

// Defining a macro to make it more comfortable to print to stdout and the output file
#define LOG(fmt...) printf(fmt); fprintf(outfile, fmt)

#define NUM_CENTROS_VACUNACION 5
#define NUM_FABRICAS 3
#define NUM_TANDAS 10

#define MIN_TIEMPO_REPARTO 1

#define MAX_LINE_LENGTH 1024

#define CLEAR_COLOR "\x1b[0m"

char* fabricasColor[NUM_FABRICAS] = {
    "\x1b[38;2;240;209;75m",
    "\x1b[38;2;27;189;207m",
    "\x1b[38;2;206;111;133m"
};

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

FILE* outfile;

// returns whether it could read the config file or not
int readConfig(char* filename);
void printConfig();

int randInt(int min, int max);

// TODO: Create structs(?) for vac. centers and suppliers
struct VacCenter {
    int numVacunas;
    int totalVacunasRecibidas;
    int totalVacunados;
};

typedef struct VacCenter vaccenter_t;

vaccenter_t centros[NUM_CENTROS_VACUNACION];


struct VacSupplier {
    pthread_t tid;

    int totalVacunasFabricadas;
    int vacunasEntregadasPorCentro[NUM_CENTROS_VACUNACION];
};

typedef struct VacSupplier vacsupplier_t;

vacsupplier_t fabricas[NUM_FABRICAS];

// TODO: Implement producer-consumer logic
void* supplier(void* args);

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

    outfile = fopen(outfileName, "w");

    LOG("VACUNACIÓN EN PANDEMIA: CONFIGURACIÓN INICIAL\n");
    printConfig();
    LOG("\nPROCESO DE VACUNACIÓN\n");

    // Seed the random number generator
    srand(time(NULL));

    for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
        centros[i] = (vaccenter_t){};
        centros[i].numVacunas = config.initialVacunas;
    }
    
    for(int i=0;i<NUM_FABRICAS;i++) {
        fabricas[i] = (vacsupplier_t){};
        pthread_create(&fabricas[i].tid, NULL, supplier, &i);
    }

    // for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
    //     centros[i] = (vaccenter_t){};
    //     centros[i].numVacunas = config.initialVacunas;
    // }
    
    for(int i=0;i<NUM_FABRICAS;i++) {
        pthread_join(fabricas[i].tid, NULL);
    }

    LOG("Vacunación finalizada\n");

    fclose(outfile);
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
        // Check if line fails to parse, using the fact that atoi returns 0 if it cant parse str into an int and checking if the str begins with '0'
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
    LOG("Habitantes: %d\nCentros de vacunación: %d\nFábricas: %d\nVacunados por tanda: %d\nVacunas iniciales en cada centro: %d\nVacunas totales por fábrica: %d\nMínimo número de vacunas fabricadas en cada tanda: %d\nMáximo número de vacunas fabricadas en cada tanda: %d\nTiempo mínimo de fabricación de una tanda de vacunas: %d\nTiempo máximo de fabricación de una tanda de vacunas: %d\nTiempo máximo de reparto de vacunas a los centros: %d\nTiempo máximo que un habitante tarda en ver que está citado para vacunarse: %d\nTiempo máximo de desplazamiento del habitante al centro de vacunación: %d\n", 
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

int getCenterWithMostDemand() {
    int minVacs = INT_MAX;
    int idx;
    for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
        if(minVacs > centros[i].numVacunas) {
            minVacs = centros[i].numVacunas;
            idx = i;
        }
    }
    return idx;
}

void* supplier(void* args) {
    int selfIdx = (*(int*)args)-1;
    int maxVacunas = config.totalHabitantes/NUM_FABRICAS;
    
    // while(fabricas[selfIdx].totalVacunasFabricadas < maxVacunas) {
    for(int xd=0;xd<3;xd++) {
        // Tiempo en fabricar
        sleep(randInt(config.minTiempoFabricacion, config.maxTiempoFabricacion));
        
        int numFab = randInt(config.minVacunasFabricadas, config.maxVacunasFabricadas);
        printf("%s", fabricasColor[selfIdx]);
        LOG("Fábrica %d prepara %d vacunas\n", selfIdx+1, numFab);
        printf("%s", CLEAR_COLOR);

        // Tiempo en repartir
        sleep(randInt(MIN_TIEMPO_REPARTO, config.maxTiempoReparto));
        
        // Reparto
        // TODO: Repartir de una forma mas mejor en base a la demanda
        int centroExcedente = getCenterWithMostDemand();
        int vacunasRepartir;
        for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
            vacunasRepartir = numFab/NUM_CENTROS_VACUNACION;
            if(i == centroExcedente) vacunasRepartir += numFab%NUM_CENTROS_VACUNACION;

            centros[i].numVacunas += vacunasRepartir;
            printf("%s", fabricasColor[selfIdx]);
            LOG("Fábrica %d entrega %d vacunas en el centro %d\n", selfIdx+1, vacunasRepartir, i+1);
            printf("%s", CLEAR_COLOR);
            
            fabricas[selfIdx].vacunasEntregadasPorCentro[i] += vacunasRepartir;
            fabricas[selfIdx].totalVacunasFabricadas += numFab;
        }
    }
    // }
    pthread_exit(0);
}
