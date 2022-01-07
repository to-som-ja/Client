#ifndef CLIENT_MRAVEC_H
#define CLIENT_MRAVEC_H

typedef struct Mravec {
    int polohaX;
    int polohaY;
    int smer; //0-hore,1-vpravo,2-dole,3-vlavo
    int logika; //0-priama,1-inverzna
} Mravec;

#endif //CLIENT_MRAVEC_H