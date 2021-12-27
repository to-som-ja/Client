#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X 30 //stlpce
#define Y 10 //riadok
#define POCET 5
typedef struct Mravec{
    int polohaX;
    int polohaY;
    int smer; //0-hore,1-vpravo,2-dole,3-vlavo
    int logika; //0-priama,1-inverzna
}Mravec;

typedef struct Plocha{
    int * velkostX;
    int * velkostY;
    int * plocha;
}Plocha;

typedef struct data{
    Plocha * pPlocha;
    int pocetM;
    Mravec * zoznamMravcov;
    //pthready


}DATA;
int mapFunction(int x,int y){
    return y*X+x;
}

void * logika(void * data){
    DATA * dataV =(DATA *) data;
    return 0;
}


void * zobraz(void * data){
    DATA * dataV =(DATA *) data;

    for (int i = 0; i < Y; ++i) {
        for (int j = 0; j < X; ++j) {
            int smerMravca = 4;//4-niejeMravec
            //printf("poloha X  %d",(dataV->zoznamMravcov[0].polohaX));

            for (int k = 0; k < dataV->pocetM; ++k) {
                //printf("poloha X  %d",(dataV->zoznamMravcov[0].polohaX));
                if(dataV->zoznamMravcov[k].polohaX == j && dataV->zoznamMravcov[k].polohaY == i){
                    //printf("tu");
                    smerMravca = dataV->zoznamMravcov[k].smer;

                }
            }

            //printf("zobrazujem smer %d",smerMravca);

            if (smerMravca!=4){

                switch (smerMravca) {
                    case 0: printf("^ ");
                        break;
                    case 1: printf("> ");
                        break;
                    case 2: printf("v ");
                        break;
                    case 3: printf("< ");
                        break;
                }
            }else{
                //printf("%d ",dataV->pPlocha->plocha[mapFunction(j,i)]);
                if(dataV->pPlocha->plocha[mapFunction(j,i)] == 0){
                   printf("_ ");
                } else{
                   printf("O ");
               }
            }

        }
        printf("\n");
    }
    return 0;
}


int main() {
    srand(time(NULL));
    int velkostX = X;
    int velkostY = Y;
    //int plocha[X][Y];

   // memset(plocha,0,X*Y);
   // plocha[2][2]=1;
    Plocha p = {&velkostX,&velkostY,malloc(sizeof(int)*X*Y)};
    memset(p.plocha,0,X*Y*sizeof(int));
    p.plocha[mapFunction(3,4)]=1;//4stlpec 5 riadok

    int pocet = POCET;
    Mravec poleMravcov[POCET];
    for (int i = 0; i < pocet; ++i) {
        int polohaX = rand()%velkostX;
        int polohaY = rand()%velkostY;
        int smer = rand()%4;
        int logika = 0;
        Mravec m = {polohaX, polohaY, smer, logika};
        poleMravcov[i] = m;
    }
    for (int i = 0; i < pocet; ++i) {

    }
    DATA d={&p,POCET,poleMravcov};
    //printf("%d",d.pPlocha->plocha[2][2]);

    zobraz(&d);

    return 0;
}

