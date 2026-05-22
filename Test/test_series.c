/*
 * test_series.c
 *
 * RÔLE : teste les fonctions de series.c
 *
 * FONCTIONNEMENT :
 *   - Contient une seule fonction publique : run_tests_series()
 *   - Appelée depuis main.c en mode test
 *   - Utilise le fichier Data/x10.dat qui doit exister dans le projet
 *
 * FONCTIONS TESTÉES :
 *   read_series(const char* filename, size_t* n)
 *     Entrée  : chemin vers un fichier .dat binaire
 *     Sortie  : tableau de shorts alloué + nombre d'échantillons dans n
 *
 *   quantize_series(short* x, size_t n)
 *     Entrée  : tableau x de n shorts
 *     Sortie  : nouveau tableau xQ où chaque valeur est paire (bit 0 mis à 0)
 */

#include "test_framework.h"
#include "series.h"

/* ── Test 1 : read_series sur Data/x10.dat
 *   On vérifie : retour non NULL, n == 10
 *   On ne peut pas vérifier les valeurs exactes sans connaître le fichier
 * ── */
static void test_read_series(void) {
    printf("\n[S1] read_series — Data/x10.dat\n");

    size_t n   = 0;
    short* x   = read_series("Data/x10.dat", &n);

    ASSERT(x  != NULL, "read_series retourne non NULL");
    ASSERT(n  == 10,   "n == 10 echantillons");

    if (x) free(x);
}

/* ── Test 2 : read_series sur fichier inexistant ── */
static void test_read_inexistant(void) {
    printf("\n[S2] read_series — fichier inexistant\n");

    size_t n   = 0;
    short* res = read_series("Data/fichier_inexistant.dat", &n);

    ASSERT(res == NULL, "retourne NULL pour fichier inexistant");
}

/* ── Test 3 : quantize_series
 *   Entrée connue : x = {100, 101, 200, 201, -1, -2, 0, 1, 32767, 32766}
 *   Résultat attendu calculé à la main :
 *     100  pair   -> 100
 *     101  impair -> 100
 *     200  pair   -> 200
 *     201  impair -> 200
 *     -1   impair -> -2
 *     -2   pair   -> -2
 *     0    pair   -> 0
 *     1    impair -> 0
 *     32767 impair-> 32766
 *     32766 pair  -> 32766
 * ── */
static void test_quantize(void) {
    printf("\n[S3] quantize_series — 10 valeurs connues\n");

    short x[]  = {100, 101, 200, 201, -1, -2, 0, 1, 32767, 32766};
    short* xQ  = quantize_series(x, 10);

    ASSERT(xQ != NULL,      "retourne non NULL");
    if (!xQ) return;

    ASSERT(xQ[0] == 100,   "xQ[0] == 100  (100 pair)");
    ASSERT(xQ[1] == 100,   "xQ[1] == 100  (101 impair -> 100)");
    ASSERT(xQ[2] == 200,   "xQ[2] == 200  (200 pair)");
    ASSERT(xQ[3] == 200,   "xQ[3] == 200  (201 impair -> 200)");
    ASSERT(xQ[4] == -2,    "xQ[4] == -2   (-1 impair -> -2)");
    ASSERT(xQ[5] == -2,    "xQ[5] == -2   (-2 pair)");
    ASSERT(xQ[6] == 0,     "xQ[6] == 0    (0 pair)");
    ASSERT(xQ[7] == 0,     "xQ[7] == 0    (1 impair -> 0)");
    ASSERT(xQ[8] == 32766, "xQ[8] == 32766 (32767 impair -> 32766)");
    ASSERT(xQ[9] == 32766, "xQ[9] == 32766 (32766 pair)");

    free(xQ);
}

/* ── Test 4 : toutes les valeurs de xQ sont paires ── */
static void test_quantize_tous_pairs(void) {
    printf("\n[S4] quantize_series — toutes valeurs paires\n");

    size_t n  = 0;
    short* x  = read_series("Data/x10.dat", &n);
    if (!x) { printf("  [--] Data/x10.dat absent, test ignore\n"); return; }

    short* xQ = quantize_series(x, n);
    ASSERT(xQ != NULL, "retourne non NULL");

    if (xQ) {
        int ok = 1;
        for (size_t i = 0; i < n; i++)
            if (xQ[i] % 2 != 0) { ok = 0; break; }
        ASSERT(ok, "toutes les valeurs de xQ sont paires");
        free(xQ);
    }
    free(x);
}

/* ── Point d'entrée appelé depuis main.c ── */
void run_tests_series(void) {
    printf("=== Tests series.c ===\n");
    test_read_series();
    test_read_inexistant();
    test_quantize();
    test_quantize_tous_pairs();
}
