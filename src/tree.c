#include "tree.h"
#include "histogram.h"
#include "display.h"
#include <stdio.h>
#include <stdlib.h>

int branches_valides = 0;
int LONGEUR_MAXIMALE = 0;

static Noeud* global_root = NULL;

//delai en ms entre chaque etape de l'elagage (pour voir l'animation)
#define DELAY_ELAGAGE_MS 5

/* Creation d'un fils */
void creer_fils(Noeud* pere, short valeur_fils, Noeud* fils) {
    fils->valeur  = valeur_fils;
    fils->pere    = pere;
    fils->filsG   = NULL;
    fils->filsD   = NULL;
    fils->dead    = 0;
    fils->solution = 0;
    //copie du Px du pere puis decrement de la transition empruntee
    fils->Px      = copy_histogram(pere->Px);
    local_decrement(fils->Px, pere->valeur, valeur_fils);
    //pair -> gauche, impair -> droite
    if (valeur_fils % 2 == 0)
        pere->filsG = fils;
    else
        pere->filsD = fils;
}

#ifndef NO_ELAGAGE
/*   Elagage recursif : supprime un noeud mort et remonte tant que
 *    le pere devient lui-meme une feuille non-solution.
 *    Active par defaut. Pour desactiver : #define NO_ELAGAGE dans tree.h
 */
static void elaguer(Noeud* noeud, int rang) {
    if (!noeud) return;
    //ne jamais elaguer la racine ni une solution
    if (noeud->pere == NULL) return;
    if (noeud->solution) return;

    Noeud* pere = noeud->pere;

    //detacher le noeud de son pere
    if (pere->filsG == noeud) pere->filsG = NULL;
    if (pere->filsD == noeud) pere->filsD = NULL;

    //liberer la memoire du noeud (et de son sous-arbre s'il en reste)
    free_tree(noeud);

#ifndef NO_DISPLAY
    //afficher la disparition du noeud avec un petit delai pour voir l'animation
    update_display((void*)global_root, rang);
    SDL_Delay(DELAY_ELAGAGE_MS);
#endif

    //si le pere se retrouve sans aucun fils et n'est pas une solution,
    //il devient lui-meme a elaguer (sauf si c'est la racine)
    if (!pere->filsG && !pere->filsD && !pere->solution && pere->pere != NULL) {
        pere->dead = 1;
#ifndef NO_DISPLAY
        //afficher le pere en rouge avant de l'elaguer a son tour
        update_display((void*)global_root, rang - 1);
        SDL_Delay(DELAY_ELAGAGE_MS);
#endif
        elaguer(pere, rang - 1);
    }
}
#endif

/* Construction recursive */
void construire(Noeud* noeud, int rang, short Xq[]) {

    //if (branches_valides >= 100) return;
    //condition d'arret : feuille finale
    if (rang == LONGEUR_MAXIMALE) {
        branches_valides++;
        noeud->solution = 1;  //marquer comme solution valide (vert dans SDL)
        fprintf(stderr, "Solution %d trouvee rang %d\n",
                branches_valides, rang);
        fflush(stderr);
        free_local_histogram(noeud->Px);
        noeud->Px = NULL;
#ifndef NO_DISPLAY
        //rafraichir pour montrer ce noeud en vert
        update_display((void*)global_root, rang);
#endif
        return;
    }

    //les deux candidats pour ce rang
    short  candidats[2]  = { Xq[rang], (short)(Xq[rang] + 1) };
    Noeud* fils_crees[2] = { NULL, NULL };
    int    nb_fils       = 0;

    //creer tous les fils valides avant de liberer le Px courant
    for (int i = 0; i < 2; i++) {
        short val_fils = candidats[i];
        if (local_get_count(noeud->Px, noeud->valeur, val_fils) > 0) {
            Noeud* fils = (Noeud*)malloc(sizeof(Noeud));
            if (!fils) { fprintf(stderr, "Erreur malloc\n"); exit(1); }
            creer_fils(noeud, val_fils, fils);
            fils_crees[nb_fils++] = fils;

#ifndef NO_DISPLAY
            //afficher le nouveau noeud cree
            update_display((void*)global_root, rang + 1);
#endif
        }
    }

    //liberer le Px du noeud courant : les fils ont leurs copies
    free_local_histogram(noeud->Px);
    noeud->Px = NULL;

    if (nb_fils == 0) {
        //aucun fils valide : noeud mort (rouge dans SDL)
        noeud->dead = 1;
#ifndef NO_DISPLAY
        //afficher le noeud en rouge avant l'elagage
        update_display((void*)global_root, rang);
        SDL_Delay(DELAY_ELAGAGE_MS);
#endif
#ifndef NO_ELAGAGE
        //elagage en remontant : supprime ce noeud mort et eventuellement
        //ses ancetres devenus feuilles non-solution
        elaguer(noeud, rang);
#endif
        return;
    }

    //recurser sur les fils
    for (int i = 0; i < nb_fils; i++)
        construire(fils_crees[i], rang + 1, Xq);
}

/* Point d'entree */
Noeud* build_tree(short Xq[], Node** Px) {
    branches_valides = 0;

    //racine fictive : valeur 0, pas de pere, pas dans le signal
    Noeud* racine = (Noeud*)malloc(sizeof(Noeud));
    if (!racine) { fprintf(stderr, "Erreur malloc racine\n"); exit(1); }
    racine->pere     = NULL;
    racine->filsG    = NULL;
    racine->filsD    = NULL;
    racine->valeur   = 0;
    racine->dead     = 0;
    racine->solution = 0;
    racine->Px       = Px;

    global_root = racine;

    //les deux premiers fils : Xq[0] (pair) et Xq[0]+1 (impair)
    //pas de decrement car la racine n'est pas un echantillon reel
    short  candidats[2]  = { Xq[0], (short)(Xq[0] + 1) };
    Noeud* fils_crees[2] = { NULL, NULL };
    int    nb_fils       = 0;

    for (int i = 0; i < 2; i++) {
        short val_fils = candidats[i];
        Noeud* fils = (Noeud*)malloc(sizeof(Noeud));
        if (!fils) { fprintf(stderr, "Erreur malloc fils\n"); exit(1); }
        fils->valeur   = val_fils;
        fils->pere     = racine;
        fils->filsG    = NULL;
        fils->filsD    = NULL;
        fils->dead     = 0;
        fils->solution = 0;
        //copie independante du Px sans decrement (rang 0, pas de predecesseur reel)
        fils->Px       = copy_histogram(racine->Px);
        if (val_fils % 2 == 0) racine->filsG = fils;
        else                   racine->filsD = fils;
        fils_crees[nb_fils++] = fils;
    }

    racine->Px = NULL;

    fprintf(stderr, "Debut build_tree n=%d\n", LONGEUR_MAXIMALE);
    fflush(stderr);

    for (int i = 0; i < nb_fils; i++)
        construire(fils_crees[i], 1, Xq);

    fprintf(stderr, "build_tree termine : %d solution(s)\n", branches_valides);
    fflush(stderr);

#ifndef NO_DISPLAY
    update_display((void*)global_root, LONGEUR_MAXIMALE);
#endif

    return racine;
}

/* Affichage des solutions */
static void print_path(Noeud* noeud) {
    if (!noeud || noeud->pere == NULL) return;
    print_path(noeud->pere);
    fprintf(stderr, "%d ", noeud->valeur);
}

void print_solutions(Noeud* root) {
    if (!root) return;
    if (root->solution) {
        fprintf(stderr, "Solution : ");
        print_path(root);
        fprintf(stderr, "\n");
        fflush(stderr);
        return;
    }
    print_solutions(root->filsG);
    print_solutions(root->filsD);
}

/* Liberation de l'arbre */
void free_tree(Noeud* node) {
    if (!node) return;
    free_tree(node->filsG);
    free_tree(node->filsD);
    if (node->Px) { free_local_histogram(node->Px); node->Px = NULL; }
    free(node);
}
