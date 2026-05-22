/*
 * test_histogram.c
 *
 * RÔLE : teste les fonctions de histogram.c
 *
 * FONCTIONNEMENT :
 *   - Contient une seule fonction publique : run_tests_histogram()
 *   - Appelée depuis main.c en mode test
 *   - Chaque test vérifie une fonction précise avec ASSERT
 *   - reset() remet hash_table à zéro entre les tests
 *
 * FONCTIONS TESTÉES :
 *   insert_couple(prev, curr)     : ajoute le couple (prev->curr) dans hash_table
 *   get_count(prev, curr)         : retourne le nombre d'occurrences dans hash_table
 *   decrement_count(prev, curr)   : décrémente le compteur dans hash_table
 *   build_histogram(short* x, n)  : construit hash_table depuis le tableau x de taille n
 *   copy_histogram(Node** src)    : retourne une copie indépendante de src
 *   local_get_count(px, prev, curr) : lit le compteur dans une copie locale px
 *   local_decrement(px, prev, curr) : décrémente dans une copie locale px
 */

#include <string.h>
#include "test_framework.h"
#include "histogram.h"
#include "tree.h"

/* Remet hash_table à zéro avant chaque test */
static void reset(void) {
    free_histogram();
    memset(hash_table, 0, sizeof(hash_table));
    branches_valides = 0;
}

/* ── Test 1 : insert_couple + get_count ── */
static void test_insert_get(void) {
    printf("\n[H1] insert_couple / get_count\n");
    reset();

    insert_couple(10, 20);
    insert_couple(10, 20);
    insert_couple(10, 20);

    ASSERT(get_count(10, 20) == 3, "get_count(10,20) == 3 apres 3 insertions");
    ASSERT(get_count(10, 21) == 0, "get_count(10,21) == 0 couple absent");
    ASSERT(get_count(20, 10) == 0, "get_count(20,10) == 0 ordre different");
}

/* ── Test 2 : decrement_count ── */
static void test_decrement(void) {
    printf("\n[H2] decrement_count\n");
    reset();

    insert_couple(10, 20);
    insert_couple(10, 20);

    decrement_count(10, 20);
    ASSERT(get_count(10, 20) == 1, "count == 1 apres 1 decrement");

    decrement_count(10, 20);
    ASSERT(get_count(10, 20) == 0, "count == 0 apres 2 decrements");

    decrement_count(10, 20);
    ASSERT(get_count(10, 20) == 0, "count reste 0, ne passe pas negatif");
}

/* ── Test 3 : build_histogram
 *   Entrée : signal connu x[] de 10 valeurs
 *   On peut compter les transitions manuellement et vérifier
 *   (10->11) apparaît 3 fois, (10->12) 2 fois, etc.
 * ── */
static void test_build(void) {
    printf("\n[H3] build_histogram\n");
    reset();

    short x[] = {10, 11, 10, 11, 10, 12, 10, 11, 12, 10};
    build_histogram(x, 10);

    ASSERT(get_count(10, 11) == 3, "get_count(10,11) == 3");
    ASSERT(get_count(11, 10) == 2, "get_count(11,10) == 2");
    ASSERT(get_count(10, 12) == 1, "get_count(10,12) == 1");
    ASSERT(get_count(12, 10) == 2, "get_count(12,10) == 2");
    ASSERT(get_count(10, 10) == 0, "get_count(10,10) == 0 transition absente");
}

/* ── Test 4 : copy_histogram — indépendance des copies
 *   Modifier la copie ne doit pas changer l'original
 * ── */
static void test_copy(void) {
    printf("\n[H4] copy_histogram\n");
    reset();

    insert_couple(10, 20);
    insert_couple(10, 20); /* count = 2 */

    Node** copie = copy_histogram(hash_table);

    local_decrement(copie, 10, 20); /* décrement dans la copie */

    ASSERT(get_count(10, 20) == 2,              "Original inchange (count == 2)");
    ASSERT(local_get_count(copie, 10, 20) == 1, "Copie decrementee (count == 1)");

    free_local_histogram(copie);
}

/* ── Point d'entrée appelé depuis main.c ── */
void run_tests_histogram(void) {
    printf("=== Tests histogram.c ===\n");
    test_insert_get();
    test_decrement();
    test_build();
    test_copy();
}
