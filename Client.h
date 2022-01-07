#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H
//
// Created by Thinkpad on 7. 1. 2022.
//

#include "Data.h"

void menu();

int mapFunction(int x, int y, int maxX);

void posunMravca(int maxX, int maxY, Mravec *m);

void *logika(void *data);

void nahodneCierne(Plocha *p);

void nastavCierne(Plocha *p);

void *vypinac(void *data);

void *zobraz(void *data);

void ulozSvetLokalne(Plocha *p);

int spojenieServer(Plocha *pPlocha, int akcia, int *velkostX, int *velkostY);

#endif //CLIENT_CLIENT_H
