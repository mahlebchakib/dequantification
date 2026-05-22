/*
 * test_integration.c
 *
 * RÔLE : teste la chaîne complète entre les modules
 *
 * FONCTIONNEMENT :
 *   - Contient une seule fonction publique : run_tests_integration()
 *   - Appelée depuis main.c en mode test
 *   - Utilise Data/x10.dat
 *
 * DIFFÉRENCE AVEC LES TESTS UNITAIRES :
 *   Les tests unitaires testent chaque fonction SEULE avec des données
 *   construites à la main en mémoire.
 *   Les tests d'intégration testent plusieurs modules ENCHAÎNÉS :
 *   on lit un vrai fichier, on construit l'histogramme, on quantifie,
 *   on construit l'arbre — et on vérifie que tout s'enchaîne sans erreur.
 *
 * DIFFÉRENCE TF-03 vs TI-01 :
 *   TF-03 (test_histogram.c) : build_histogram() reçoit un tableau
 *   construit directement en mémoire dans le test.
 *   TI-01 (ici) : le tableau vient de read_series() — on teste que
 *   le résultat de series.c est bien accepté par histogram.c.
 */

#include <string.h>
#include "test_framework.h"
#include "histogram.h"
#include "series.h"
#include "tree.h"

static void reset(void) {
    free_histogram();
    memset(hash_table, 0, sizeof(hash_table));
}

/* ── TI-01 : read_series -> build_histogram
 *   On lit Data/x10.dat et on vérifie que l'histogramme
 *   est bien construit depuis les données lues (pas depuis
 *   un tableau en mémoire comme dans TF-03).
 * ── */
static void test_ti01_series_vers_histo(void) {
    printf("\n[I1] read_series -> build_histogram (Data/x10.dat)\n");
    reset();

    size_t n = 0;
    short* x = read_series("Data/x10.dat", &n);

    if (!x) {
        printf("  [--] Data/x10.dat absent, test ignore\n");
        return;
    }

    ASSERT(n == 10, "n == 10 echantillons lus");

    /* transmettre directement à build_histogram sans modifier x */
    build_histogram(x, n);

    /* l'histogramme doit contenir au moins une transition */
    int au_moins_une = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] != NULL) { au_moins_une = 1; break; }
    }
    ASSERT(au_moins_une, "histogramme non vide apres build_histogram(lu)");

    free(x);
    reset();
}

/* ── TI-02 : hash_table intact après build_tree ── */
static void test_ti02_histo_intact(void) {
    printf("\n[I2] hash_table inchange apres build_tree\n");
    reset();

    short x[]  = {10, 10, 10};
    short xQ[] = {10, 10, 10};

    build_histogram(x, 3);
    short avant = get_count(10, 10);

    Node** Px = copy_histogram(hash_table);
    LONGEUR_MAXIMALE = 3;
    Noeud* root = build_tree(xQ, Px);

    ASSERT(get_count(10, 10) == avant,
        "hash_table inchange apres build_tree");
    ASSERT(branches_valides >= 1,
        "au moins 1 solution trouvee");

    free_tree(root);
    reset();
}

/* ── TI-03 : chaîne complète sur Data/x10.dat ── */
static void test_ti03_chaine_complete(void) {
    printf("\n[I3] Chaine complete : read -> histo -> quantize -> build_tree\n");
    reset();

    size_t n = 0;
    short* x = read_series("Data/x10.dat", &n);

    if (!x) {
        printf("  [--] Data/x10.dat absent, test ignore\n");
        return;
    }

    build_histogram(x, n);
    Node** Px = copy_histogram(hash_table);
    short* xQ = quantize_series(x, n);

    LONGEUR_MAXIMALE = (int)n;
    Noeud* root = build_tree(xQ, Px);

    ASSERT(root             != NULL, "racine non NULL");
    ASSERT(branches_valides >= 1,    "au moins 1 solution trouvee");

    free_tree(root);
    free(x);
    free(xQ);
    reset();
}

/* ── TI-04 : aucun Px résiduel après build_tree ── */
static void test_ti04_pas_px_residuel(void) {
    printf("\n[I4] Aucun Px residuel apres build_tree\n");
    reset();

    short x[]  = {10, 10, 10, 10, 10};
    short xQ[] = {10, 10, 10, 10, 10};

    build_histogram(x, 5);
    Node** Px = copy_histogram(hash_table);
    LONGEUR_MAXIMALE = 5;
    Noeud* root = build_tree(xQ, Px);

    /* parcourir l'arbre et vérifier qu'aucun noeud n'a de Px */
    int px_residuel = 0;
    Noeud* pile[512];
    int top = 0;
    pile[top++] = root;
    while (top > 0) {
        Noeud* cur = pile[--top];
        if (!cur) continue;
        if (cur->Px != NULL) px_residuel++;
        if (top < 510) {
            pile[top++] = cur->filsG;
            pile[top++] = cur->filsD;
        }
    }
    ASSERT(px_residuel == 0, "aucun Px residuel dans l arbre");

    free_tree(root);
    reset();
}

/* ── Point d'entrée appelé depuis main.c ── */
void run_tests_integration(void) {
    printf("=== Tests d integration ===\n");
    test_ti01_series_vers_histo();
    test_ti02_histo_intact();
    test_ti03_chaine_complete();
    test_ti04_pas_px_residuel();
}
