#ifndef ALLOCATOR_HEAP_H
#define ALLOCATOR_HEAP_H

#include <stdio.h>

struct memory_manager_t {
    void *memory_start;
    size_t memory_size;
    struct memory_chunk_t *first_memory_chunk;
};

struct memory_chunk_t {
    struct memory_chunk_t *prev;
    struct memory_chunk_t *next;
    size_t size;
    size_t validation_value;
};
enum pointer_type_t {
    pointer_null,
    pointer_heap_corrupted,
    pointer_control_block,
    pointer_inside_fences,
    pointer_inside_data_block,
    pointer_unallocated,
    pointer_valid
};

int heap_setup(void);

void heap_clean(void);

void *heap_malloc(size_t size);

void *heap_calloc(size_t number, size_t size);

void *heap_realloc(void *memblock, size_t size);

int heap_validate(void);

size_t heap_get_largest_used_block_size(void);

enum pointer_type_t get_pointer_type(const void *const pointer);

void heap_free(void *memblock);

void add_fences(struct memory_chunk_t *chunk);

size_t value_of_chunk(struct memory_chunk_t *chunk);

#endif //ALLOCATOR_HEAP_H
