#ifndef PRECISION_H
#define PRECISION_H

struct Noeud;

void   construire_series(struct Noeud* branche, short* xR);
double calc_precision(const short* x, const short* xR, int n);
double calc_precision_noeud(struct Noeud* branche, const short* original_x, int n);

#endif