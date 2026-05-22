#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <stddef.h>

#define HASH_SIZE 10007

typedef struct Node {
    short prev;
    short curr;
    short count;
    struct Node* next;
} Node;

extern Node* hash_table[HASH_SIZE];

unsigned int hash_couple(short prev, short curr);
void         insert_couple(short prev, short curr);
void         build_histogram(short* x, size_t n);
short        get_count(short prev, short curr);
void         decrement_count(short prev, short curr);
void         print_histogram(void);
void         free_histogram(void);

/* Nouvelles fonctions pour Px local */
Node**       copy_histogram(Node** src);
void         free_local_histogram(Node** px);
short        local_get_count(Node** px,
                              short prev,
                              short curr);
void         local_decrement(Node** px,
                              short prev,
                              short curr);

#endif
