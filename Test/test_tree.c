/*
 * test_tree.c
 *
 * RÔLE : teste les fonctions de tree.c
 *
 * FONCTIONNEMENT :
 *   - Contient une seule fonction publique : run_tests_tree()
 *   - Appelée depuis main.c en mode test
 *   - Compiler avec -DNO_DISPLAY pour désactiver SDL
 *
 * FONCTIONS TESTÉES :
 *   creer_fils(Noeud* pere, short valeur_fils, Noeud* fils)
 *     Entrée  : père existant avec Px alloué, valeur du fils, noeud fils alloué
 *     Effet   : initialise fils, copie Px du père, décrémente la transition,
 *               attache fils à pere->filsG (pair) ou pere->filsD (impair)
 *
 *   construire(Noeud* noeud, int rang, short Xq[])
 *     Entrée  : noeud courant avec Px alloué, rang actuel, tableau Xq
 *     Effet   : crée les fils valides, libère Px du noeud,
 *               marque dead=1 si aucun fils valide
 *
 *   build_tree(short Xq[], Node** Px)
 *     Entrée  : tableau Xq quantifié, histogramme Px
 *     Sortie  : racine de l'arbre construit
 *
 *   free_tree(Noeud* node)
 *     Entrée  : racine de l'arbre
 *     Effet   : libère tous les noeuds récursivement
 */

#include <string.h>
#include "test_framework.h"
#include "tree.h"
#include "histogram.h"

static void reset(void) {
    free_histogram();
    memset(hash_table, 0, sizeof(hash_table));
}

/* Alloue un noeud minimal pour les tests */
static Noeud* alloc_noeud(short valeur) {
    Noeud* n  = malloc(sizeof(Noeud));
    n->valeur = valeur;
    n->pere   = NULL;
    n->filsG  = NULL;
    n->filsD  = NULL;
    n->dead   = 0;
    n->Px     = NULL;
    return n;
}

/* ── Test 1 : creer_fils — valeur paire va à gauche ── */
static void test_creer_fils_gauche(void) {
    printf("\n[T1] creer_fils — valeur paire -> filsG\n");
    reset();

    insert_couple(10, 20);
    insert_couple(10, 20);
    insert_couple(10, 20); /* count = 3 */

    Noeud* pere = alloc_noeud(10);
    pere->Px    = copy_histogram(hash_table);
    Noeud* fils = malloc(sizeof(Noeud));

    creer_fils(pere, 20, fils);

    ASSERT(fils->valeur == 20,   "fils->valeur == 20");
    ASSERT(fils->pere   == pere, "fils->pere == pere");
    ASSERT(pere->filsG  == fils, "pere->filsG == fils (20 pair)");
    ASSERT(pere->filsD  == NULL, "pere->filsD == NULL");
    ASSERT(local_get_count(fils->Px, 10, 20) == 2,
        "Px fils decremente : count == 2");
    ASSERT(local_get_count(pere->Px, 10, 20) == 3,
        "Px pere intact : count == 3");

    free_local_histogram(fils->Px);
    free_local_histogram(pere->Px);
    free(fils);
    free(pere);
}

/* ── Test 2 : creer_fils — valeur impaire va à droite ── */
static void test_creer_fils_droite(void) {
    printf("\n[T2] creer_fils — valeur impaire -> filsD\n");
    reset();

    insert_couple(10, 21);
    Noeud* pere = alloc_noeud(10);
    pere->Px    = copy_histogram(hash_table);
    Noeud* fils = malloc(sizeof(Noeud));

    creer_fils(pere, 21, fils);

    ASSERT(pere->filsD == fils, "pere->filsD == fils (21 impair)");
    ASSERT(pere->filsG == NULL, "pere->filsG == NULL");

    free_local_histogram(fils->Px);
    free_local_histogram(pere->Px);
    free(fils);
    free(pere);
}

/* ── Test 3 : construire — noeud sans transition valide -> dead=1 ── */
static void test_noeud_mort(void) {
    printf("\n[T3] construire — noeud mort (dead=1)\n");
    reset();

    /* aucune transition depuis la valeur 50 */
    insert_couple(1, 2);

    LONGEUR_MAXIMALE = 10;
    branches_valides = 0;

    Noeud* noeud = alloc_noeud(50);
    noeud->Px    = copy_histogram(hash_table);

    short Xq[10] = {0};
    Xq[6] = 100; /* candidats 100 et 101 absents du Px */

    construire(noeud, 5, Xq);

    ASSERT(noeud->dead      == 1, "dead == 1");
    ASSERT(branches_valides == 0, "branches_valides == 0");

    free(noeud);
}

/* ── Test 4 : construire — feuille au rang final -> branches_valides++ ── */
static void test_feuille_valide(void) {
    printf("\n[T4] construire — feuille au rang final\n");
    reset();

    LONGEUR_MAXIMALE = 5;
    branches_valides = 0;

    Noeud* noeud = alloc_noeud(10);
    noeud->Px    = copy_histogram(hash_table);

    short Xq[5] = {0};
    construire(noeud, 5, Xq); /* rang == LONGEUR_MAXIMALE  */

    ASSERT(branches_valides == 1, "branches_valides == 1");
    ASSERT(noeud->Px        == NULL, "Px libere apres feuille");

    free(noeud);
}

/* ── Test 5 : build_tree sur signal simple ── */
static void test_build_tree(void) {
    printf("\n[T5] build_tree — signal pair n=3\n");
    reset();

    /* x = {10,10,10} : transition (10->10) presente 2 fois */
    short x[]  = {10, 10, 10};
    short xQ[] = {10, 10, 10};

    build_histogram(x, 3);
    Node** Px = copy_histogram(hash_table);
    LONGEUR_MAXIMALE = 3;

    Noeud* root = build_tree(xQ, Px);

    ASSERT(root             != NULL, "racine non NULL");
    ASSERT(root->pere       == NULL, "root->pere == NULL (racine fictive)");
    ASSERT(root->valeur     == 0,    "root->valeur == 0");
    ASSERT(branches_valides >= 1,    "au moins 1 solution trouvee");

    free_tree(root);
    reset();
}

/* ── Point d'entrée appelé depuis main.c ── */
void run_tests_tree(void) {
    printf("=== Tests tree.c ===\n");
    test_creer_fils_gauche();
    test_creer_fils_droite();
    test_noeud_mort();
    test_feuille_valide();
    test_build_tree();
}
