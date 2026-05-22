#ifndef DISPLAY_H
#define DISPLAY_H
//#define NO_DISPLAY // décommenter pour enlever l'affichage graphique du programme

#include "tree.h"
#include <SDL2/SDL.h>

#define WIN_W       1200
#define WIN_H        700
#define NODE_R        16
#define LEVEL_H       85
#define BTN_W         110
#define BTN_H         32
#define BTN_Y         70
#define BTN_PAUSE_X   20
#define BTN_RELANCE_X 140
#define BTN_QUITTER_X 260

//delai en ms apres creation d'un noeud (vitesse de l'animation)
#define DELAY_NODE_MS 20

void init_display(void);
void update_display(void* root, int current_rank);
void display_final(void* root, int nb_solutions);
void close_display(void);
//retourne 1 si l'utilisateur a clique sur RELANCER pendant display_final, 0 sinon
int display_relance_demandee(void);

#endif
