// définitions et prototypes

#ifndef SERIES_H_
#define SERIES_H_

#include <stddef.h>

// Lire une série depuis un fichier .dat
short* read_series(const char* filename, size_t* num_samples);

// Sous-quantifier une série x sur 1 bits
short* quantize_series(short* x, size_t n);

// Affichage de la série
void afficher_serie(short* x, size_t n);


#endif /* SERIES_H_ */
