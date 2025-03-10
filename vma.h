#pragma once
#include <inttypes.h>
#include <stddef.h>



typedef struct miniblock_t {
uint64_t start_address;
size_t size;
uint8_t perm;
void *rw_buffer;
int verify;
} miniblock_t;

typedef struct node node;
struct node {
void *data;
node *prev, *next;
};

typedef struct list_t {
node *head;
unsigned int data_size;
unsigned int size;
} list_t;

typedef struct block_t {
uint64_t start_address;
uint64_t size;
list_t *miniblock_list;
} block_t;

typedef struct {
uint64_t arena_size, free_size;
list_t *alloc_list;
int aloc, mini;
} arena_t;
