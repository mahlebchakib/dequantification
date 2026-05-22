// fonctions pour lecture et quantification

#include "series.h"
#include <stdio.h>
#include <stdlib.h>

//Lécture de la série depuis un fichier binaire
short* read_series(const char* filename, size_t* num_samples) {
    FILE *f = fopen(filename, "rb");
    if (!f) { printf("Erreur ouverture fichier\n"); return NULL; }

    fseek(f, 0, SEEK_END); //Positionner le marqueur en fin de fichier
    long filesize = ftell(f); //Récuperer la taille en octet
    rewind(f); //Remettre le marqueur en  début de fichier

    *num_samples = filesize / sizeof(short);
    short* data = malloc(*num_samples * sizeof(short));
    if (!data) { printf("Erreur allocation mémoire\n"); fclose(f); return NULL; }

    size_t lus = fread(data, sizeof(short), *num_samples, f);
    if (lus != *num_samples) {
        fprintf(stderr, "Erreur lecture : %zu/%zu echantillons lus\n",
                lus, *num_samples);
        free(data);
        fclose(f);
        return NULL;
    }    fclose(f);
    return data;
}

//Quantification de 1 bit de la série original Xn et récuperer Xq en sortie
short* quantize_series(short* x, size_t n) {
    short* xQ = malloc(n * sizeof(short));
    if (!xQ) { printf("Erreur allocation mémoire\n"); return NULL; }

    for (size_t i = 0; i < n; i++) {
        xQ[i] = (x[i] >> 1) << 1;  /*Décallage binaire de 1 bit vers la droite
                                    puis 1 bit vers la gauche pour perdre 1 bit de précision*/
    }
    return xQ;
}

// Affichage de la série
void afficher_serie(short* serie, size_t n) {
    if (serie == NULL) {
        printf("Tableau NULL\n");
        return;
    }

    printf("xN : \n");
    printf("[ ");
    for (size_t i = 0; i < n; i++) {
        printf("%hd ", serie[i]);
    }
    printf("]\n");
}
