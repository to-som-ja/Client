#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define X 30 //stlpce
#define Y 10 //riadok
#define POCET 10
typedef struct Mravec {
    int polohaX;
    int polohaY;
    int smer; //0-hore,1-vpravo,2-dole,3-vlavo
    int logika; //0-priama,1-inverzna
} Mravec;

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
    pthread_mutex_t *mutex;
    pthread_cond_t *vypocitane;
    pthread_cond_t *vykreslene;

} DATA;

int mapFunction(int x, int y) {
    return y * X + x;
}

void posunMravca (int maxX, int maxY, Mravec *m) {

    switch (m->smer) {
        case 0:
            if (m->polohaY - 1 >= 0) {
                m->polohaY--;
            }
            break;
        case 1:
            if (m->polohaX + 1 < maxX) {
                m->polohaX++;
            }
            break;
        case 2:
            if (m->polohaY + 1 < maxY) {
                m->polohaY++;
            }
            break;
        case 3:
            if (m->polohaX - 1 >= 0) {
                m->polohaX--;
            }
            break;
    }

}

// logika mravca
void *logika(void *data) {
    printf("Zacinam pocitat\n");
    DATA *dataV = (DATA *) data;
    pthread_mutex_lock(dataV->mutex);
    while (dataV->vykresluje) {
        pthread_cond_wait(dataV->vypocitane, dataV->mutex);
    }
    for (int i = 0; i < dataV->pocetM; ++i) {
        Mravec *mravec = &dataV->zoznamMravcov[i];
        int polohaX = mravec->polohaX;
        int polohaY = mravec->polohaY;
        int *farbaPolicka = &dataV->pPlocha->plocha[mapFunction(polohaX, polohaY)];

        // inverzna logika
        if(mravec->logika) {
            // cierne policko
            if (*farbaPolicka) {
                mravec->smer = (mravec->smer + 1) % 4;
                *farbaPolicka = 1;
            }
            // biele policko
            else {
                mravec->smer = (mravec->smer + 3) % 4;
                *farbaPolicka = 0;
            }
        }
        // priama logika
        else {
            // biele policko
            if (!(*farbaPolicka)) {
                mravec->smer = (mravec->smer + 1) % 4;
                *farbaPolicka = 1;
            }
            // cierne policko
            else {
                mravec->smer = (mravec->smer + 3) % 4;
                *farbaPolicka = 0;
            }
        }
        posunMravca(*dataV->pPlocha->velkostX, *dataV->pPlocha->velkostY, mravec);
    }
    dataV->vykresluje = 1;
    pthread_cond_signal(dataV->vykreslene);
    printf("Koncim pocitanie\n");
    pthread_mutex_unlock(dataV->mutex);
    return 0;
}

void nahodneCierne (Plocha *p) {
    for (int i = 0; i < *p->velkostX; ++i) {
        for (int j = 0; j < *p->velkostY; ++j) {
            int sancaCierne = rand() % 100;
            if (sancaCierne < 30) {
                p->plocha[mapFunction(i,j)] = 1;
            }
        }
    }
}

void nastavCierne (Plocha *p) {
    while (1) {
        printf("Na ukoncenie zadajte cislo mimo rozsahu.\n");
        int vstupX = 0;
        int vstupY = 0;
        printf("Zadajte X suradnicu <0, %d) : ", *p->velkostX);
        scanf("%d", &vstupX);
        printf("\nZadajte Y suradnicu <0, %d) : ", *p->velkostY);
        scanf("%d", &vstupY);
        printf("\n");
        if (vstupX >= 0 && vstupX < *p->velkostX && vstupY >= 0 && vstupY < *p->velkostY) {
            p->plocha[mapFunction(vstupX, vstupY)] = 1;
        } else {
            printf("Koncim nastavovanie.\n");
            break;
        }
    }
}

void *zobraz(void *data) {
    DATA *dataV = (DATA *) data;
    pthread_mutex_lock(dataV->mutex);
    while (!dataV->vykresluje) {
        pthread_cond_wait(dataV->vykreslene, dataV->mutex);
    }
    printf("Zobrazujem\n");
    for (int i = 0; i < *dataV->pPlocha->velkostY; ++i) {
        for (int j = 0; j < *dataV->pPlocha->velkostX; ++j) {
            int smerMravca = 4;         // 4 - na policku nie je mravec

            for (int k = 0; k < dataV->pocetM; ++k) {
                if (dataV->zoznamMravcov[k].polohaX == j && dataV->zoznamMravcov[k].polohaY == i) {
                    smerMravca = dataV->zoznamMravcov[k].smer;

                }
            }

            int farbaPolicka = dataV->pPlocha->plocha[mapFunction(j,i)];
            if (smerMravca != 4) {
                switch (smerMravca) {
                    case 0:
                        if (farbaPolicka) {
                            printf("▲ ");
                        } else {
                            printf("^ ");
                        }
                        break;
                    case 1:
                        if (farbaPolicka) {
                            printf("► ");
                        } else {
                            printf("> ");
                        }
                        break;
                    case 2:
                        if (farbaPolicka) {
                            printf("▼ ");
                        } else {
                            printf("v ");
                        }
                        break;
                    case 3:
                        if (farbaPolicka) {
                            printf("◄ ");
                        } else {
                            printf("< ");
                        }
                        break;
                }
            } else {
                if (farbaPolicka == 0) {
                    printf("□ ");
                } else {
                    printf("■ ");
                }
            }

        }
        printf("\n");
    }
    dataV->vykresluje = 0;
    pthread_cond_signal(dataV->vypocitane);
    printf("Koncim zobrazovanie\n");
    pthread_mutex_unlock(dataV->mutex);
    return 0;
}

// TODO
/*
 * Logika mravca (priama a inverzna)
 *
 */
int main() {
    srand(time(NULL));
    // TODO
    /*
     * Sparametrizovat
     */
    int velkostX = X;
    int velkostY = Y;
    int pocet = POCET;
    int vykresluje = 1;
    pthread_t grafikaTH, logikaTH;
    pthread_mutex_t mutex;
    pthread_cond_t vykreslene, vypocitane;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&vykreslene, NULL);
    pthread_cond_init(&vypocitane, NULL);

    Plocha p = {&velkostX, &velkostY, malloc(sizeof(int) * velkostX * velkostY)};
    memset(p.plocha, 0, velkostX * velkostY * sizeof(int));
//    nahodneCierne(&p);
    nastavCierne(&p);

    DATA d = {&p, pocet, malloc(sizeof(Mravec) * pocet), vykresluje, &mutex, &vykreslene, &vypocitane};

    for (int i = 0; i < pocet; ++i) {
        int polohaX = rand() % velkostX;
        int polohaY = rand() % velkostY;
        int smer = rand() % 4;

        int logika = 0;
        Mravec m = {polohaX, polohaY, smer, logika};
        d.zoznamMravcov[i] = m;
    }
    pthread_create(&grafikaTH, NULL, &zobraz, &d);
    pthread_create(&logikaTH, NULL, &logika, &d);

    pthread_join(grafikaTH, NULL);
    pthread_join(logikaTH, NULL);


    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&vypocitane);
    pthread_cond_destroy(&vykreslene);
    return 0;
}

