#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>

/*
 * Variables globales qui comptent les résultats.
 * Définies ici une seule fois.
 * Tous les fichiers de test les utilisent via #include.
 */
extern int tests_passes;
extern int tests_echoues;

/*
 * ASSERT(condition, message)
 * Si condition est vraie  -> affiche [OK] et incrémente tests_passes
 * Si condition est fausse -> affiche [KO] et incrémente tests_echoues
 */
#define ASSERT(condition, message)                 \
    do {                                           \
        if (condition) {                           \
            printf("  [OK] %s\n", message);        \
            tests_passes++;                        \
        } else {                                   \
            printf("  [KO] %s\n", message);        \
            tests_echoues++;                       \
        }                                          \
    } while (0)

/* Prototypes des suites de tests — définies dans chaque fichier test_xxx.c */
void run_tests_histogram(void);
void run_tests_series(void);
void run_tests_tree(void);
void run_tests_integration(void);

#endif
