#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define X 30 //stlpce
#define Y 10 //riadok
#define POCET 10
#define POCET_AKCII 12

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
    int akcieStret;
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
    while (1) {
        pthread_mutex_lock(dataV->mutex);
        while (dataV->vykresluje) {
            pthread_cond_wait(dataV->vypocitane, dataV->mutex);
        }
        for (int i = 0; i < dataV->pocetM; ++i) {
            Mravec *mravec = &dataV->zoznamMravcov[i];
            int polohaX = mravec->polohaX;
            int polohaY = mravec->polohaY;
            int *farbaPolicka = &dataV->pPlocha->plocha[mapFunction(polohaX, polohaY)];
/* TODO
 * akcia pri strete viacerych mravcov 0/1/2
 */
            // inverzna logika
            if (mravec->logika) {
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
            int cisloPrveho;
            int cisloDruheho;
            int nachadzaSaInyMravec=0;
            for (int j = 0; j < dataV->pocetM; ++j) {
                if(dataV->zoznamMravcov[i].polohaX==polohaX && dataV->zoznamMravcov[i].polohaY==polohaY)
                {
                    if (nachadzaSaInyMravec==0){
                        cisloPrveho=j;
                    }else{
                        cisloDruheho = j;
                    }
                    nachadzaSaInyMravec++;
                }

            }
            if(nachadzaSaInyMravec>1){
                switch (dataV->akcieStret) {
                    case 0:
                        dataV->zoznamMravcov[cisloDruheho]=dataV->zoznamMravcov[dataV->pocetM];
                        dataV->pocetM--;
                        dataV->zoznamMravcov[cisloPrveho]=dataV->zoznamMravcov[dataV->pocetM];
                        dataV->pocetM--;
                        break;
                    case 1:
                        //memmove(&dataV->zoznamMravcov[cisloDruheho],&dataV->zoznamMravcov[dataV->pocetM],sizeof (Mravec));
                        dataV->zoznamMravcov[cisloDruheho]=dataV->zoznamMravcov[dataV->pocetM];
                            dataV->pocetM--;
                        break;
                    case 2:

                        break;
                }
            }
        }
        dataV->vykresluje = 1;
        pthread_cond_signal(dataV->vykreslene);
        printf("Koncim pocitanie\n");
        pthread_mutex_unlock(dataV->mutex);
    }
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
    while (1) {
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

                int farbaPolicka = dataV->pPlocha->plocha[mapFunction(j, i)];
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
    }
    return 0;
}

void vytvorSvet(Plocha *pPlocha) {
    int rozmerX, rozmerY;
    printf("Zadajte X rozmer sveta: ");
    scanf("%d", &rozmerX);
    printf("\nZadajte Y rozmer sveta: ");
    scanf("%d", &rozmerY);
    printf("\n");

    pPlocha->velkostY = &rozmerY;
    pPlocha->velkostX = &rozmerX;
    pPlocha->plocha = malloc(sizeof(int) * rozmerX * rozmerY);
    memset(pPlocha->plocha, 0, rozmerX * rozmerY * sizeof(int));
}


// TODO
/*
 * Logika mravca (priama a inverzna)
 *
 */
int main() {
    srand(time(NULL));
    char vstup;
    Plocha p;
    int pocet;
    DATA d;
    d.akcieStret = 0;

    int vykresluje = 1;
    pthread_t grafikaTH, logikaTH;
    pthread_mutex_t mutex;
    pthread_cond_t vykreslene, vypocitane;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&vykreslene, NULL);
    pthread_cond_init(&vypocitane, NULL);


    char textAkcii[POCET_AKCII][36] = {"1 - Vytvorit svet", "2 - Generovat cierne polia", "3 - Definovat konkretne cierne polia",
                                       "4 - Nacitat svet (lokalne)", "5 - Ulozit svet (lokalne)", "6 - Pocet mravcov", "7 - Logika mravcov",
                                       "8 - Akcia pri strete mravcov", "9 - Spustit / Zastavit simulaciu", "10 - Nacitat svet (server)",
                                       "11 - Ulozit svet (server)", "X - Ukoncit aplikaciu"};
    int dostupneAkcie[POCET_AKCII] = {1,0,0,1,0,0,0,0,0,1,0,1}; //bitova reprezentacia dostupnych akcii
    while (strcmp(vstup, "X") != 0) {
        printf("---MENU---\nMozne akcie:\n");
        for (int i = 0; i < POCET_AKCII; ++i) {
            if(dostupneAkcie[i]) {
                printf("%s", textAkcii[i]);
                printf("\n");
            }
        }
        printf("Vyberam akciu cislo: ");
        scanf("%c", vstup);
        if (strcmp(vstup, "X") == 0) {
            ukonciAplikaciu();
        }
        int akcia = atoi(&vstup);
        switch (akcia) {
            /* TODO
             * Skontrolovat bitovu reprezentaciu pri kazdom case!!!
             */
            case 1:
                if (dostupneAkcie[0]) {
                    vytvorSvet(&p);
                    int dostupneAkcie2[12] = {0,1,1,0,1,1,0,0,0,0,1,1};
                    memcpy(dostupneAkcie, dostupneAkcie2, sizeof (dostupneAkcie));

                }
                break;
            case 2:
                if (dostupneAkcie[1]) {
                    nahodneCierne(&p);
                }
                break;
            case 3:
                if (dostupneAkcie[2]) {
                    nastavCierne(&p);
                }
                break;
            case 4:
                if (dostupneAkcie[3]) {
                    // nacitajSvetLokalne(&p);
                }
                break;
            case 5:
                if (dostupneAkcie[4]) {
                    // ulozSvetLokalne(&p);
                }
                break;
            case 6:
                if (dostupneAkcie[5]) {
                    printf("Zadajte pocet mravcov: ");
                    scanf("%d", &pocet);
                    d.pocetM = pocet;
                    d.zoznamMravcov = malloc(sizeof(Mravec) * pocet);
                    for (int i = 0; i < pocet; ++i) {
                        int polohaX = rand() % *p.velkostX;
                        int polohaY = rand() % *p.velkostY;
                        int smer = rand() % 4;

                        int logika = 0;
                        Mravec m = {polohaX, polohaY, smer, logika};
                        d.zoznamMravcov[i] = m;
                    }
                }
                break;
            case 7:
                if (dostupneAkcie[6]) {
                    int logika;
                    printf("Vyberte logiku mravcov (0 = priama / 1 = inverzna): ");
                    scanf("%d", &logika);
                    for (int i = 0; i < pocet; ++i) {
                        d.zoznamMravcov[i].logika = logika;
                    }
                }
                break;
            case 8:
                if (dostupneAkcie[7]) {
                    int akciaStret;
                    printf("Vyberte ako sa budu mravce spravat pri strete (0 = zanik vsetkych stretnutych mravcov / 1 = prezije jeden mravec / 2 = polovica mravcov sa zacne spravat podla doplnkovej logiky): ");
                    scanf("%d", &akciaStret);
                    d.akcieStret = akciaStret;
                }
                break;
            case 9:
                if (dostupneAkcie[8]) {
                    zapniVypni();
                }
                break;
            case 10:
                if (dostupneAkcie[9]) {
                    nacitajSvetZoServera();
                }
                break;
            case 11:
                if (dostupneAkcie[10]) {
                    ulozSvetNaServer();
                }
                break;
        }
        d.pPlocha = &p;
        //  = {&p, pocet, malloc(sizeof(Mravec) * pocet), vykresluje, &mutex, &vykreslene, &vypocitane}
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


