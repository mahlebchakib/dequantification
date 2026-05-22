#include <stdio.h>
#include <stdlib.h>
#include "precision.h"
#include "tree.h"

/* Remonte la branche depuis la feuille jusqu'a la racine fictive
   pour remplir xR dans l'ordre chronologique. */
static int path_length(Noeud* node) {
    int len = 0;
    while (node->pere) {
        len++;
        node = node->pere;
    }
    return len;
}

void construire_series(Noeud* branche, short* xR) {
    int i = path_length(branche) - 1; /* -1 pour ignorer la racine fictive */
    Noeud* current = branche;
    while (current->pere) {
        xR[i] = current->valeur;
        current = current->pere;
        i--;
    }
}

/* Calcul du taux de reussite entre deux tableaux de meme taille */
double calc_precision(const short* x, const short* xR, int n) {
    if (!x || !xR || n <= 0) return 0.0;
    int correct = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] == xR[i])
            correct++;
    }
    return ((double)correct / n) * 100.0;
}

/* Calcul de precision depuis un noeud solution de l'arbre */
double calc_precision_noeud(Noeud* branche, const short* original_x, int n) {
    if (!branche || n <= 0) return 0.0;

    short* xR = (short*)malloc(n * sizeof(short));
    if (!xR) return 0.0;

    construire_series(branche, xR);
    double prec = calc_precision(original_x, xR, n);
    free(xR);
    return prec;
}