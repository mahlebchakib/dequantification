#include "histogram.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* hash_table[HASH_SIZE]; //Table de hashage

//Fonction de hashage
unsigned int hash_couple(short prev, short curr) {
    return ((unsigned int)prev * 2654435761u //pour avoir une bonne répartition des couples
            + (unsigned int)curr) % HASH_SIZE;
}

//Insertion des couples dans la table de hashage
void insert_couple(short prev, short curr) {
    unsigned int h    = hash_couple(prev, curr);
    Node*        node = hash_table[h];
    while (node) {
        if (node->prev == prev
            && node->curr == curr) {
            node->count++;
            return;
        }
        node = node->next;
    }
    Node* n   = malloc(sizeof(Node));
    n->prev   = prev;
    n->curr   = curr;
    n->count  = 1;
    n->next   = hash_table[h];
    hash_table[h] = n;
}

//Construction de l'histogramme à partir de la série original lue
void build_histogram(short* x, size_t n) {
    memset(hash_table, 0, sizeof(hash_table)); /*Fonction de la lib string.h qui
                               	   	   initialiose toute les case de la tables à 0 octet */
    for (size_t i = 1; i < n; i++)
        insert_couple(x[i-1], x[i]);
}

//Récuperer le nombre de fois qu'un couple apparait dans l'histogramme (pour les tests dans test_histogram(2).c))
short get_count(short prev, short curr) {
    unsigned int h    = hash_couple(prev, curr);
    Node*        node = hash_table[h];
    while (node) {
        if (node->prev == prev
            && node->curr == curr)
            return node->count;
        node = node->next;
    }
    return 0;
}

//Decrémenter le nombre de fois qu'un couple apparait dans l'histogramme (pour les tests dans test_histogram(2).c))
void decrement_count(short prev, short curr) {
    unsigned int h    = hash_couple(prev, curr);
    Node*        node = hash_table[h];
    while (node) {
        if (node->prev == prev
            && node->curr == curr) {
            if (node->count > 0)
                node->count--;
            return;
        }
        node = node->next;
    }
}

//Affichage de l'histogramme
void print_histogram(void) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Node* node = hash_table[i];
        while (node) {
            printf("(%d,%d) = %d\n",
                   node->prev,
                   node->curr,
                   node->count);
            node = node->next;
        }
    }
}

//Libérer la mémoire allouée pour l'histogramme
void free_histogram(void) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Node* node = hash_table[i];
        while (node) {
            Node* tmp = node;
            node      = node->next;
            free(tmp);
        }
        hash_table[i] = NULL;
    }
}


//Copie complète d'une table de hachage
Node** copy_histogram(Node** src) {
	Node** dst = calloc(HASH_SIZE, sizeof(Node*));
	if (!dst) {
        fprintf(stderr, "ERREUR copy_histogram\n");
        fflush(stderr);
        exit(1);
    }
    for (int i = 0; i < HASH_SIZE; i++) {
        dst[i]      = NULL;
        Node*  node = src[i];
        Node** tail = &dst[i];
        while (node) {
            Node* n   =(Node*) malloc(sizeof(Node));
            if (!n) {
                fprintf(stderr,
                    "ERREUR malloc copy\n");
                fflush(stderr);
                exit(1);
            }
            n->prev   = node->prev;
            n->curr   = node->curr;
            n->count  = node->count;
            n->next   = NULL;
            *tail      = n;
            tail       = &n->next;
            node       = node->next;
        }
    }
    return dst;
}

//Libère une table de hachage locale
void free_local_histogram(Node** px) {
    if (!px) return;
    for (int i = 0; i < HASH_SIZE; i++) {
        Node* node = px[i];
        while (node) {
            Node* tmp = node;
            node      = node->next;
            free(tmp);
        }
    }
    free(px);
}

//Consulte un Px local
short local_get_count(Node** px,
                       short prev, short curr) {
    if (!px) return 0;
    unsigned int h    = hash_couple(prev, curr);
    Node*        node = px[h];
    while (node) {
        if (node->prev == prev
            && node->curr == curr)
            return node->count;
        node = node->next;
    }
    return 0;
}

//Décrémente dans un Px local
void local_decrement(Node** px,
                      short prev, short curr) {
    if (!px) return;
    unsigned int h    = hash_couple(prev, curr);
    Node*        node = px[h];
    while (node) {
        if (node->prev == prev
            && node->curr == curr) {
            if (node->count > 0)
                node->count--;
            return;
        }
        node = node->next;
    }
}
