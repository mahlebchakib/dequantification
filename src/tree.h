#ifndef TREE_H
#define TREE_H
//#define NO_ELAGAGE //décommenter pour enlever l'élagage récursif
#include <stdio.h>
#include <stdlib.h>
#include "histogram.h"

extern int LONGEUR_MAXIMALE;

typedef struct Noeud {
    short  valeur;
    Node** Px;
    //1 si feuille valide (rang final atteint) -> affiché vert
    int    solution;
    //1 si feuille morte (aucun fils valide) -> affiché rouge
    int    dead;
    struct Noeud* pere;
    struct Noeud* filsD;
    struct Noeud* filsG;
} Noeud;

extern int branches_valides;

void   creer_fils(Noeud* pere, short valeur_fils, Noeud* fils);
void   construire(Noeud* noeud, int rang, short Xq[]);
Noeud* build_tree(short Xq[], Node** Px);
void   free_tree(Noeud* node);
void   print_solutions(Noeud* root);

#endif
