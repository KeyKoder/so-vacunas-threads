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
#define MIN_TIEMPO_REACCION 1
#define MIN_TIEMPO_DESPLAZAMIENTO 1

#define MAX_LINE_LENGTH 1024

#define CLEAR_COLOR "\x1b[0m"

// REFERENCIA!!1!1
char* fabricasColor[NUM_FABRICAS] = {
    "\x1b[38;2;240;209;75m", // NERU
    "\x1b[38;2;27;189;207m", // MIKU
    "\x1b[38;2;206;111;133m" // TETO
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
    pthread_t tid;

    pthread_mutex_t mutex;
    pthread_mutex_t mutexFabricas;

    int numVacunas;

    int habitantesEnCola;

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

pthread_t* habitantes;

// TODO: Implement producer-consumer logic
void* supplier(void* args);
void* habitante(void* args);

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

    habitantes = malloc(config.totalHabitantes/NUM_TANDAS*sizeof(pthread_t));


    for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
        centros[i] = (vaccenter_t){};
        centros[i].numVacunas = config.initialVacunas;
        pthread_mutex_init(&centros[i].mutex, NULL);
        pthread_mutex_init(&centros[i].mutexFabricas, NULL);
        // pthread_create(&centros[i].tid, NULL, centro, &i);
    }
    
    for(int i=0;i<NUM_FABRICAS;i++) {
        fabricas[i] = (vacsupplier_t){};
        pthread_create(&fabricas[i].tid, NULL, supplier, (void*)&i);
    }

    int numHab = -1;
    for(int t=0;t<NUM_TANDAS;t++) {
        for(int i=0;i<config.totalHabitantes/NUM_TANDAS;i++) {
            numHab++;
            pthread_create(habitantes+i, NULL, habitante, (void*)&numHab);
        }
        for(int i=0;i<config.totalHabitantes/NUM_TANDAS;i++) {
            pthread_join(habitantes[i], NULL);
        }
    }

    for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
        // pthread_join(centros[i].tid, NULL);
        pthread_mutex_destroy(&centros[i].mutex);
        pthread_mutex_destroy(&centros[i].mutexFabricas);
    }
    
    for(int i=0;i<NUM_FABRICAS;i++) {
        pthread_join(fabricas[i].tid, NULL);
    }

    LOG("Vacunación finalizada\n");

    for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
        LOG("El centro %d ha recibido %d vacunas, ha vacunado a %d habitantes y le han sobrado %d vacunas\n",
            i+1,
            centros[i].totalVacunasRecibidas,
            centros[i].totalVacunados,
            centros[i].numVacunas);
    }
    
    for(int i=0;i<NUM_FABRICAS;i++) {
        LOG("La fábrica %d ha fabricado %d vacunas y las ha repartido de la siguiente forma:\n", i+1, fabricas[i].totalVacunasFabricadas);
        for(int c=0;c<NUM_CENTROS_VACUNACION;c++) {
            LOG("Centro %d -> %d vacunas\n", c+1, fabricas[i].vacunasEntregadasPorCentro[c]);
        }
    }

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

int getCenterWithLeastVaccines() {
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
    int selfIdx = (*(int*)args);
    int maxVacunas = config.totalHabitantes/NUM_FABRICAS;
    
    while(fabricas[selfIdx].totalVacunasFabricadas < maxVacunas) {
        // Tiempo en fabricar
        sleep(randInt(config.minTiempoFabricacion, config.maxTiempoFabricacion));
        
        int numFab;
        // Evitar que se pase
        if(maxVacunas - fabricas[selfIdx].totalVacunasFabricadas < config.minVacunasFabricadas) {
            numFab = maxVacunas - fabricas[selfIdx].totalVacunasFabricadas;
        }else {
            numFab = randInt(config.minVacunasFabricadas, config.maxVacunasFabricadas);
        }

        // Evitar que se pase con el aleatorio
        if(numFab + fabricas[selfIdx].totalVacunasFabricadas > maxVacunas) {
            numFab = maxVacunas - fabricas[selfIdx].totalVacunasFabricadas;
        }

        printf("%s", fabricasColor[selfIdx]);
        LOG("Fábrica %d prepara %d vacunas\n", selfIdx+1, numFab);
        printf("%s", CLEAR_COLOR);

        // Calcular cuantas vamos a repartir
        // TODO: Repartir en base a demanda
        float totalEnCola = 0;
        float totalReparto = 0;
        int colas[NUM_CENTROS_VACUNACION]; // debug
        int reparto[NUM_CENTROS_VACUNACION];
        int centerExcedentIdx = getCenterWithLeastVaccines(); // Para cuando no haya nadie en cola en ninguno
        for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
            totalEnCola += centros[i].habitantesEnCola;
            colas[i] = centros[i].habitantesEnCola;
        }
        
        for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
            if(totalEnCola != 0) {
                reparto[i] = (int)(numFab*(centros[i].habitantesEnCola/totalEnCola));


                // Si está cerca de la mitad, redondear hacia arriba
                // if((numFab*(centros[i].habitantesEnCola/totalEnCola)) - reparto[i] >= 0.45 && (numFab*(centros[i].habitantesEnCola/totalEnCola)) - reparto[i] <= 0.55) {
                //     reparto[i]++;
                // }
            }else {
                // Si no hay gente en cola, se lo damos al que menos vacunas tenga
                reparto[i] = numFab/NUM_CENTROS_VACUNACION;
                if(i == centerExcedentIdx) reparto[i] += numFab%NUM_CENTROS_VACUNACION;
            }
            totalReparto += reparto[i];
        }

        if(totalReparto < numFab) {
            reparto[centerExcedentIdx] += numFab - totalReparto;
        }
        
        // Mantener un mínimo de 1 repartido por centro
        // if(totalEnCola > NUM_CENTROS_VACUNACION) {
        //     totalEnCola -= NUM_CENTROS_VACUNACION; 
        //     numFab -= NUM_CENTROS_VACUNACION;
        // }

        // Tiempo en repartir
        sleep(randInt(MIN_TIEMPO_REPARTO, config.maxTiempoReparto));
        
        // Reparto
        // TODO: Repartir de una forma mas mejor en base a la demanda
        for(int i=0;i<NUM_CENTROS_VACUNACION;i++) {
            // Saca el porcentaje de las vacunas fabricadas a darle a este centro (redondeado hacia arriba)
            
            printf("%s", fabricasColor[selfIdx]);
            printf("\x1b[48;2;50;50;50mFábrica %d va a entregar %d vacunas en el centro %d (%.3f, %.3f, %d)\x1b[0m\n", selfIdx+1, reparto[i], 
                i+1, colas[i]/totalEnCola, (numFab*(colas[i]/totalEnCola)), (int)totalEnCola);
            printf("%s", CLEAR_COLOR);
            

            pthread_mutex_lock(&centros[i].mutexFabricas);

            centros[i].numVacunas += reparto[i];
            printf("%s", fabricasColor[selfIdx]);
            LOG("Fábrica %d entrega %d vacunas en el centro %d\n", selfIdx+1, reparto[i], i+1);
            printf("%s", CLEAR_COLOR);
            
            fabricas[selfIdx].vacunasEntregadasPorCentro[i] += reparto[i];
            centros[i].totalVacunasRecibidas += reparto[i];

            pthread_mutex_unlock(&centros[i].mutexFabricas);
        }
        fabricas[selfIdx].totalVacunasFabricadas += numFab;
    }
    pthread_exit(0);
}

void* habitante(void* args) {
    int selfIdx = (*(int*)args);

    // Tiempo en enterarse
    sleep(randInt(MIN_TIEMPO_REACCION, config.maxTiempoReaccion));

    // Elige centro
    int centroIdx = randInt(0, NUM_CENTROS_VACUNACION-1);
    LOG("Habitante %d elige el centro %d para vacunarse\n", selfIdx+1, centroIdx+1);

    // Tiempo en llegar al centro
    sleep(randInt(MIN_TIEMPO_DESPLAZAMIENTO, config.maxTiempoDespl));
    
    centros[centroIdx].habitantesEnCola++;
    
    printf("\x1b[38;2;255;150;250mHabitante %d esperando, son %d en la cola del centro %d\x1b[0m\n", selfIdx+1, centros[centroIdx].habitantesEnCola, centroIdx+1);

    
    pthread_mutex_lock(&centros[centroIdx].mutex);

    // Espera hasta que haya vacunas
    while(centros[centroIdx].numVacunas <= 0) {
        sleep(1);
    }
    
    // Se vacuna
    centros[centroIdx].numVacunas--;
    centros[centroIdx].totalVacunados++;
    centros[centroIdx].habitantesEnCola--;
    LOG("Habitante %d vacunado en el centro %d\n", selfIdx+1, centroIdx+1);

    pthread_mutex_unlock(&centros[centroIdx].mutex);

    pthread_exit(0);
}
