#include <stdio.h>
#include <stdlib.h>
#include "series.h"
#include "histogram.h"
#include "tree.h"
#include "display.h"
#include "precision.h"


int main(int argc, char* argv[]) {

    //Verification arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier.dat>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    size_t n_samples;
    short* x = read_series(filename, &n_samples);

    if (!x || n_samples == 0) {
        fprintf(stderr, "Erreur lecture serie\n");
        return 1;
    }
    printf("Nombre d'echantillons : %lu\n", (unsigned long)n_samples);

    //affichage de la serie
    afficher_serie(x, n_samples);

    //quantification : calcul de xQ depuis x
    short* xQ = quantize_series(x, n_samples);
    if (!xQ) {
        fprintf(stderr, "Erreur quantification\n");
        free(x);
        return 1;
    }
    printf("Quantification terminee\n");

    //affichage de xQ
    printf("xQ : \n");
    printf("[ ");
    for (size_t i = 0; i < n_samples; i++) printf("%d ", xQ[i]);
    printf("]\n");

    //construire l'histogramme de la serie originale (une seule fois)
    build_histogram(x, n_samples);
    printf("Histogramme des couples :\n");
    print_histogram();

    //initialisation de la longueur maximale pour construire()
    LONGEUR_MAXIMALE = (int)n_samples;

    //boucle principale : tant que l'utilisateur clique RELANCER, on recommence
    do {
        //copie fraiche de l'histogramme a chaque iteration
        //(build_tree consomme Px en le decrementant, donc il faut une copie neuve)
        Node** Px = copy_histogram(hash_table);

        //initialisation de l'affichage SDL
        #ifndef NO_DISPLAY
        init_display();
        #endif

        //construction de l'arbre de dequantification
        fprintf(stderr, "Construction de l'arbre en cours...\n");
        Noeud* root = build_tree(xQ, Px);

        if (!root) {
            fprintf(stderr, "Erreur construction arbre\n");
            #ifndef NO_DISPLAY
            close_display();
            #endif
            free(x);
            free(xQ);
            free_histogram();
            return 1;
        }

        //affichage des solutions trouvees
        if (branches_valides > 0) {
            fprintf(stderr, "\n=== %d solution(s) ===\n", branches_valides);
            print_solutions(root);
            // Calcul de la precision pour chaque solution
                        fprintf(stderr, "\n=== Precision des solutions ===\n");
                        // Parcours de l'arbre pour trouver les noeuds solutions
                        // On utilise une pile manuelle pour eviter la recursion
                        Noeud* stack[4096];
                        int top = 0;
                        stack[top++] = root;
                        int num_sol = 1;
                        while (top > 0) {
                            Noeud* cur = stack[--top];
                            if (!cur) continue;
                            if (cur->solution) {
                                double prec = calc_precision_noeud(cur, x, (int)n_samples);
                                fprintf(stderr, "Solution %d : precision = %.2f%%\n", num_sol++, prec);
                            }
                            if (cur->filsD) stack[top++] = cur->filsD;
                            if (cur->filsG) stack[top++] = cur->filsG;
                        }


        } else {
            fprintf(stderr, "Aucune solution trouvee\n");
        }

        //affichage final SDL (attend la fermeture de la fenetre ou un clic RELANCER/QUITTER)
        #ifndef NO_DISPLAY
        display_final((void*)root, branches_valides);
        #endif

        //nettoyage de l'arbre et fermeture de SDL avant relance eventuelle
        free_tree(root);
        #ifndef NO_DISPLAY
        close_display();
        #endif

    } while (display_relance_demandee());

    //nettoyage final apres sortie de la boucle
    free(x);
    free(xQ);
    free_histogram();

    return 0;
}
