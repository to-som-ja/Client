#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define POCET_AKCII 12
#define PORT 11123
#define IP_ADDRESS "localhost"

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
    int stoj;
    int akcieStret;
    pthread_mutex_t *mutex;
    pthread_cond_t *vypocitane;
    pthread_cond_t *vykreslene;
    pthread_cond_t *stoji;

} DATA;


int mapFunction(int x, int y, int maxX) {
    return y * maxX + x;
}

void posunMravca(int maxX, int maxY, Mravec *m) {

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
    DATA *dataV = (DATA *) data;
    printf("Logika\n");
    while (1) {
        pthread_mutex_lock(dataV->mutex);

        while (dataV->vykresluje) {
            pthread_cond_wait(dataV->vypocitane, dataV->mutex);
        }
        //printf("Pocitam\n");

        for (int i = 0; i < dataV->pocetM; ++i) {
            Mravec *mravec = &dataV->zoznamMravcov[i];
            int polohaX = mravec->polohaX;
            int polohaY = mravec->polohaY;
            int *farbaPolicka = &dataV->pPlocha->plocha[mapFunction(polohaX, polohaY,*dataV->pPlocha->velkostX)];

            // inverzna logika
            if (mravec->logika) {
                // cierne policko
                if (*farbaPolicka) {
                    mravec->smer = (mravec->smer + 1) % 4;
                    *farbaPolicka = 0;
                }
                    // biele policko
                else {
                    mravec->smer = (mravec->smer + 3) % 4;
                    *farbaPolicka = 1;
                }
            }
                // priama logika
            else {
                // biele policko
                if (!*farbaPolicka) {
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
            //stret
            int cisloPrveho;
            int cisloDruheho;
            int nachadzaSaInyMravec=0;
            for (int j = 0; j < dataV->pocetM; ++j) {
                if(dataV->zoznamMravcov[j].polohaX==polohaX && dataV->zoznamMravcov[j].polohaY==polohaY)
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
                        for (int j = 0; j < dataV->pocetM/2; ++j) {
                            if ( dataV->zoznamMravcov[j].logika){
                                dataV->zoznamMravcov[j].logika=0;
                            }else{
                                dataV->zoznamMravcov[j].logika=1;
                            }
                        }
                        break;
                }
            }


        }
       // printf("Koncim pocitanie\n");
        dataV->vykresluje = 1;
        pthread_cond_signal(dataV->vykreslene);

        while (!dataV->stoj) {
            printf("CAKAM logika\n");
            pthread_cond_wait(dataV->stoji, dataV->mutex);

        }
        pthread_mutex_unlock(dataV->mutex);
    }

    return 0;
}

void nahodneCierne(Plocha *p) {
    for (int i = 0; i < *p->velkostX; ++i) {
        for (int j = 0; j < *p->velkostY; ++j) {
            int sancaCierne = rand() % 100;
            if (sancaCierne < 30) {
                p->plocha[mapFunction(i, j,*p->velkostX)] = 1;
            }
        }
    }
    printf("Cierne polia sa nastavili");
}

void nastavCierne(Plocha *p) {
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
            p->plocha[mapFunction(vstupX, vstupY, *p->velkostX)] = 1;
        } else {
            printf("Koncim nastavovanie.\n");
            break;
        }
    }
    printf("Cierne polia sa nastavili");
}

void *vypinac(void *data) {
    DATA *dataV = (DATA *) data; //stoj na zaciatku = 1

    while (1) {
        char str[50];
        gets(str);
        //printf("som za gets");
        pthread_mutex_lock(dataV->mutex);
        if (dataV->stoj) {
            dataV->stoj = 0;
        } else {
            pthread_cond_broadcast(dataV->stoji);
            dataV->stoj = 1;
        }
        pthread_mutex_unlock(dataV->mutex);

    }

    return 0;
}


void *zobraz(void *data) {
    DATA *dataV = (DATA *) data;
    pthread_mutex_lock(dataV->mutex);
    printf("Zobrazovac\n");

    while (1) {


        while (!dataV->vykresluje) {
            pthread_cond_wait(dataV->vykreslene, dataV->mutex);
        }
       // printf("Zobrazujem");
        for (int i = 0; i < *dataV->pPlocha->velkostY; ++i) {
            for (int j = 0; j < *dataV->pPlocha->velkostX; ++j) {
                int smerMravca = 4;         // 4 - na policku nie je mravec

                for (int k = 0; k < dataV->pocetM; ++k) {
                    if (dataV->zoznamMravcov[k].polohaX == j && dataV->zoznamMravcov[k].polohaY == i) {
                        smerMravca = dataV->zoznamMravcov[k].smer;

                    }
                }

                int farbaPolicka = dataV->pPlocha->plocha[mapFunction(j, i,*dataV->pPlocha->velkostX)];
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
        printf("\n");
        sleep(2);
        dataV->vykresluje = 0;
        pthread_cond_signal(dataV->vypocitane);
        while (!dataV->stoj) {
            printf("CAKAM zobraz\n");
            pthread_cond_wait(dataV->stoji, dataV->mutex);

        }
        pthread_mutex_unlock(dataV->mutex);
    }
    return 0;
}
void ulozSvetLokalne(Plocha *p){
    printf("Zadaj nazov suboru :");
   char nazov[20];
   scanf("%19s",nazov);
    FILE *file = fopen(nazov,"w");
    //char text[p->velkostX*p->velkostY+10];

    fprintf(file,"%d \n%d \n",*p->velkostX,*p->velkostY);
    fclose(file);
    FILE *filea = fopen(nazov,"a");
    for (int i = 0; i < *p->velkostX*(*p->velkostY); ++i) {
        fprintf(filea,"%d \n",p->plocha[i]);
    }
    fclose(filea);
}

void nacitajSvetLokalne(Plocha *pPlocha) {
    //printf("Zadaj nazov suboru :");
    //char nazov[20];
    //scanf("%19s",nazov);
    /*FILE *file = fopen("test.txt","r");
    int velkostX,velkostY;
    fscanf(file,"%d ",&velkostX);
    fscanf(file,"%d ",&velkostY);


    fclose(file);*/
}

// akcia = 0-uloz na server, 1-nacitaj zo servera
int spojenieServer(Plocha *pPlocha, int akcia) {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    server = gethostbyname(IP_ADDRESS);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(PORT);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting to socket");
        return 4;
    }

    int converted_numberDotaz =  htonl(akcia);
    write(sockfd, &converted_numberDotaz, sizeof(converted_numberDotaz));

    char buffer[256];
    bzero(buffer,256);
    printf("Zadajte nazov suboru: ");
    scanf("%255s", buffer);
    printf("Zadali ste: %s \n", buffer);
    write(sockfd, buffer, strlen(buffer));

    int status;
    if (!akcia) {
        // ukladanie suboru na server
        int a = *pPlocha->velkostX;
        int b = *pPlocha->velkostY;
        int converted_numberA = htonl(a);
        int converted_numberB = htonl(b);

        write(sockfd, &converted_numberA, sizeof(converted_numberA));
        write(sockfd, &converted_numberB, sizeof(converted_numberB));
        for (int i = 0; i < a * b; ++i) {

            int converted_numberPole = htonl(pPlocha->plocha[i]);
            write(sockfd, &converted_numberPole, sizeof(converted_numberPole));
        }
    } else {
        // Nacitanie mapy zo servera
        read(sockfd, &status, sizeof(status));
        if (ntohl(status)==0){
            int predX;
            int predY;

            read(sockfd, &predX, sizeof(predX));
            read(sockfd, &predY, sizeof(predY));

            *pPlocha->velkostX = ntohl(predX);
            *pPlocha->velkostY = ntohl(predY);
            pPlocha->plocha = malloc(sizeof (int) * *pPlocha->velkostX * *pPlocha->velkostY);
            memset(pPlocha->plocha, 0, sizeof (int) * *pPlocha->velkostX * *pPlocha->velkostY);

            for (int i = 0; i < *pPlocha->velkostX * *pPlocha->velkostY; ++i) {
                int farba;
                n = read(sockfd, &farba, sizeof(farba));
                pPlocha->plocha[i] = ntohl(farba);
            }

        } else {
            printf("cele zle-subor sa nenasiel\n");
        }
    }

    read(sockfd,&status,sizeof (status));
    status= ntohl(status);
    printf("status: %d\n",status);
    close(sockfd);
    return 0;
}

int main() {
    srand(time(NULL));


    int vstup=1;
    Plocha p;
    int pocet=1;
    DATA d;

    pthread_t grafikaTH, logikaTH, vypinacTH;
    pthread_mutex_t mutex;
    pthread_cond_t vykreslene, vypocitane, stojime;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&vykreslene, NULL);
    pthread_cond_init(&vypocitane, NULL);
    pthread_cond_init(&stojime, NULL);


    int rozmerX, rozmerY;
    d.pocetM=1;
    d.vykresluje=1;
    d.stoj=1;
    d.akcieStret = 0;
    d.mutex = &mutex;
    d.vypocitane = &vypocitane;
    d.vykreslene = &vykreslene;
    d.stoji = &stojime;
/*
    int a = 30;
    int b = 10;
    p.velkostX=&a;
    p.velkostY=&b;
    p.plocha = malloc(sizeof(int) * 30 * 10);
    memset(p.plocha, 0, 30 * 10 * sizeof(int));
    d.pPlocha = &p;
    pocet=5;
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
    pthread_create(&grafikaTH, NULL, &zobraz, &d);
    pthread_create(&logikaTH, NULL, &logika, &d);
    pthread_create(&vypinacTH, NULL, &vypinac, &d);

    pthread_join(grafikaTH, NULL);
    pthread_join(logikaTH, NULL);
    pthread_join(vypinacTH, NULL);
*/




    char textAkcii[POCET_AKCII][36] = {"1 - Vytvorit svet", "2 - Generovat cierne polia", "3 - Definovat konkretne cierne polia",
                                       "4 - Nacitat svet (lokalne)", "5 - Ulozit svet (lokalne)", "6 - Pocet mravcov", "7 - Logika mravcov",
                                       "8 - Akcia pri strete mravcov", "9 - Spustit / Zastavit simulaciu", "10 - Nacitat svet (server)",
                                       "11 - Ulozit svet (server)", "99 - Ukoncit aplikaciu"};
    int dostupneAkcie[POCET_AKCII] = {1,0,0,1,0,0,0,0,0,1,0,1}; //bitova reprezentacia dostupnych akcii
    while (vstup != 99) {
        printf("---MENU---\nMozne akcie:\n");
        for (int i = 0; i < POCET_AKCII; ++i) {
            if(dostupneAkcie[i]) {
                printf("%s", textAkcii[i]);
                printf("\n");
            }
        }
        printf("Vyberam akciu cislo: ");
        scanf("%d", &vstup);
        if (vstup == 99) {
           // ukonciAplikaciu();
        }
        switch (vstup) {

            case 1:
                if (dostupneAkcie[0]) {

                    printf("Zadajte X rozmer sveta: ");
                    scanf("%d", &rozmerX);
                    printf("\nZadajte Y rozmer sveta: ");
                    scanf("%d", &rozmerY);
                    printf("\n");

                    p.velkostY = &rozmerY;
                    p.velkostX = &rozmerX;
                    p.plocha = malloc(sizeof(int) * rozmerX * rozmerY);
                    memset(p.plocha, 0, rozmerX * rozmerY * sizeof(int));
                    d.pPlocha = &p;
                    int dostupneAkcie2[12] = {0,1,1,1,1,1,1,1,0,1,1,1};
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
                    // Nacitanie suboru
                    printf("Zadaj nazov suboru :");
                    char nazov[20];
                    scanf("%19s",nazov);
                    FILE *file = fopen(nazov,"r");
                    int velkostX,velkostY;
                    fscanf(file,"%d ",&velkostX);
                    fscanf(file,"%d ",&velkostY);


                    p.velkostY = &velkostY;
                    p.velkostX = &velkostX;
                    p.plocha = malloc(sizeof(int) * rozmerX * rozmerY);
                    memset(p.plocha, 0, rozmerX * rozmerY * sizeof(int));
                    d.pPlocha = &p;
                    for (int i = 0; i < velkostX*velkostY; ++i) {
                        int farba;
                        fscanf(file,"%d ",&farba);
                        p.plocha[i]=farba;
                    }
                    fclose(file);
                    int dostupneAkcie2[12] = {0,1,1,1,1,1,1,1,0,1,1,1};
                    memcpy(dostupneAkcie, dostupneAkcie2, sizeof (dostupneAkcie));
                }
                break;
            case 5:
                if (dostupneAkcie[4]) {
                    // Ukladanie suboru
                    ulozSvetLokalne(&p);
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
                    int dostupneAkcie2[12] = {0,1,1,1,1,1,1,1,1,0,1,1};
                    memcpy(dostupneAkcie, dostupneAkcie2, sizeof (dostupneAkcie));
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
                   // zapniVypni();
                    char str[50];
                    gets(str);
                    pthread_create(&grafikaTH, NULL, &zobraz, &d);
                    pthread_create(&logikaTH, NULL, &logika, &d);
                    pthread_create(&vypinacTH, NULL, &vypinac, &d);

                    pthread_join(grafikaTH, NULL);
                    pthread_join(logikaTH, NULL);
                    pthread_join(vypinacTH, NULL);

                }
                break;
            case 10:
                if (dostupneAkcie[9]) {
                    int *velkostX;
                    int *velkostY;
                    p.velkostX = velkostX;
                    p.velkostY = velkostY;
                    spojenieServer(&p, 1);
                    int dostupneAkcie2[12] = {0,1,1,0,1,1,1,1,1,0,1,1,};
                    memcpy(dostupneAkcie, dostupneAkcie2, sizeof (dostupneAkcie));
                }
                break;
            case 11:
                if (dostupneAkcie[10]) {
                    spojenieServer(&p, 0);
                    int dostupneAkcie2[12] = {0,1,1,0,1,1,1,1,1,0,1,1,};
                    memcpy(dostupneAkcie, dostupneAkcie2, sizeof (dostupneAkcie));
                }
                break;
        }
    }



    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&vypocitane);
    pthread_cond_destroy(&vykreslene);
    return 0;
}
/*TODO
 * nastavenie akcii
 * SERVER vsetko
 * pauza - doplnit akcie
 * uprava kodu
 * dokumentacia + prirucka
 */


