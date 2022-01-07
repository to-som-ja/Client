#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include <pthread.h>
#include "Mravec.h"

typedef struct Plocha {
    int *velkostX;
    int *velkostY;
    int *plocha;        // 1 = cierna, 0 = biela
} Plocha;

typedef struct data {
    Plocha *pPlocha;
    int pocetM;
    Mravec *zoznamMravcov;
    int vykresluje;
    int stoj;
    int akcieStret;
    pthread_mutex_t *mutex;
    pthread_cond_t *vypocitane;
    pthread_cond_t *vykreslene;
    pthread_cond_t *stoji;

} DATA;

#endif //CLIENT_DATA_H
