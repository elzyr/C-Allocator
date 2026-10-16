#include "heap.h"
#include <stdlib.h>
#include "custom_unistd.h"

typedef struct memory_chunk_t chunk_t;

# define FENCE '#'
#define FENCE_SIZE 8

struct memory_manager_t memory_manager;

size_t get_validation_value(chunk_t* chunk){
    size_t value = 0;
    value += (size_t)chunk->prev;
    value += (size_t)chunk->next;
    value += chunk->size;
    return value;
}

void update_validation(chunk_t* chunk){
    chunk->validation_value = get_validation_value(chunk);
    if (chunk->next != NULL)chunk->next->validation_value = get_validation_value(chunk->next);
    if (chunk->prev != NULL)chunk->prev->validation_value = get_validation_value(chunk->prev);
}

void* chunk_to_address(chunk_t* chunk){
    return (char*)chunk + sizeof(chunk_t) + FENCE_SIZE;
}

size_t resize_heap(int more_size){
    void* allocate = custom_sbrk(more_size);
    if (allocate == (void*)-1)return 0;
    memory_manager.memory_size += more_size;
    return 1;
}

int heap_setup(void){
    void* allocate = custom_sbrk(4096);
    if (allocate == (void*)-1)return -1;
    memory_manager.first_memory_chunk = NULL;
    memory_manager.memory_start = allocate;
    memory_manager.memory_size = 4096;
    return 0;
}

void heap_clean(void){
    resize_heap(-memory_manager.memory_size);
    memory_manager.first_memory_chunk = NULL;
}

void unlink_chunk(chunk_t* chunk){
    if (chunk == NULL)return;
    if (chunk->next != NULL) chunk->next->prev = chunk->prev;
    if (chunk->prev != NULL) chunk->prev->next = chunk->next;
    else memory_manager.first_memory_chunk = chunk->next;
    update_validation(chunk);
}

size_t get_size_between(chunk_t* first_chunk, chunk_t* second_chunk){
    char* first_chunk_memory = first_chunk != NULL
                                   ? ((char*)first_chunk + (first_chunk->size + sizeof(chunk_t) + 2 * FENCE_SIZE))
                                   : memory_manager.memory_start;
    char* second_chunk_memory = second_chunk != NULL
                                    ? (char*)second_chunk
                                    : ((char*)memory_manager.memory_start + memory_manager.memory_size);
    return second_chunk_memory - first_chunk_memory;
}

chunk_t* apply_fence(chunk_t* chunk){
    char* fence_start = (char*)chunk + sizeof(chunk_t);
    for (int i = 0; i < FENCE_SIZE; i++){
        fence_start[i] = FENCE;
        fence_start[i + chunk->size + FENCE_SIZE] = FENCE;
    }
    update_validation(chunk);
    return chunk;
}

void* heap_malloc(size_t size){
    if (size < 1)return NULL;

    //what if first is freed?
    chunk_t* memory_chunk = memory_manager.first_memory_chunk;
    chunk_t* last_chunk = NULL;
    while (memory_chunk != NULL &&
        get_size_between(last_chunk, memory_chunk) < size + sizeof(chunk_t) + FENCE_SIZE * 2){
        last_chunk = memory_chunk;
        memory_chunk = memory_chunk->next;
    }

    char* new_addr = (last_chunk == NULL)
                         ? (char*)memory_manager.memory_start
                         : (char*)last_chunk + sizeof(chunk_t) + FENCE_SIZE * 2 + last_chunk->size;

    size_t used_memory = new_addr - (char*)memory_manager.memory_start;
    size_t needed_chunk_size = sizeof(chunk_t) + size + 2 * FENCE_SIZE;
    size_t needed_total = used_memory + needed_chunk_size;
    if (needed_total > memory_manager.memory_size && !resize_heap(needed_total - memory_manager.memory_size))
        return NULL;

    chunk_t* new_chunk = (chunk_t*)new_addr;
    new_chunk->prev = last_chunk;
    new_chunk->next = memory_chunk;
    new_chunk->size = size;
    if (last_chunk == NULL) memory_manager.first_memory_chunk = new_chunk;
    else last_chunk->next = new_chunk;
    if (memory_chunk != NULL)memory_chunk->prev = new_chunk;
    return chunk_to_address(apply_fence(new_chunk));
}

chunk_t* find_chunk(void* address){
    chunk_t* memory_chunk = memory_manager.first_memory_chunk;
    while (memory_chunk != NULL && chunk_to_address(memory_chunk) != address)memory_chunk = memory_chunk->next;
    return memory_chunk;
}

void heap_free(void* address){
    unlink_chunk(find_chunk(address));
}

void* heap_calloc(size_t number, size_t size){
    char* memory = heap_malloc(number * size);
    if (memory == NULL)return NULL;
    for (size_t i = 0; i < number * size; i++)memory[i] = 0;
    return memory;
}

void* heap_realloc(void* memblock, size_t size){
    if (heap_validate() != 0)return NULL;
    if (memblock == NULL)return heap_malloc(size);

    chunk_t* memory_chunk = find_chunk(memblock);
    if (memory_chunk == NULL)return NULL;
    if (size == memory_chunk->size)return memblock;
    if (size == 0){
        heap_free(memblock);
        return NULL;
    }

    // case where we need more
    size_t size_to_next_block = get_size_between(memory_chunk, memory_chunk->next);
    if (size_to_next_block + memory_chunk->size >= size){
        memory_chunk->size = size;
        return chunk_to_address(apply_fence(memory_chunk));
    }

    // if last
    if (memory_chunk->next == NULL){
        size_t needed_size = size - (size_to_next_block + memory_chunk->size);
        if (!resize_heap(needed_size))return NULL;
        memory_chunk->size = size;
        return chunk_to_address(apply_fence(memory_chunk));
    }

    // case where we need to relocate
    char* new_add = heap_malloc(size);
    if (new_add == NULL)return NULL;
    for (size_t i = 0; i < memory_chunk->size; ++i)new_add[i] = ((char*)memblock)[i];
    heap_free(memblock);
    return new_add;
}

size_t heap_get_largest_used_block_size(void){
    if (memory_manager.memory_size < 1 || heap_validate() != 0)return 0;

    chunk_t* memory_chunk = memory_manager.first_memory_chunk;
    if (memory_chunk == NULL)return 0;

    chunk_t* largest_chunk = memory_chunk;
    while (memory_chunk != NULL){
        if (memory_chunk->size > largest_chunk->size)largest_chunk = memory_chunk;
        memory_chunk = memory_chunk->next;
    }
    return largest_chunk->size;
}

enum pointer_type_t get_pointer_type(const void* const pointer){
    if (pointer == NULL)return pointer_null;
    char* pointer_mem = (char*)pointer;
    chunk_t* memory_chunk = memory_manager.first_memory_chunk;
    while (memory_chunk != NULL){
        char* checker = (char*)memory_chunk;
        if (pointer_mem >= checker && pointer_mem < checker + sizeof(chunk_t))return pointer_control_block;
        checker += sizeof(chunk_t);
        if (pointer_mem >= checker && pointer_mem < checker + FENCE_SIZE) return pointer_inside_fences;
        checker += FENCE_SIZE;
        if (pointer_mem == checker)return pointer_valid;
        if (pointer_mem > checker && pointer_mem < checker + memory_chunk->size) return pointer_inside_data_block;
        checker += memory_chunk->size;
        if (pointer_mem >= checker && pointer_mem < checker + FENCE_SIZE) return pointer_inside_fences;
        memory_chunk = memory_chunk->next;
    }
    return pointer_unallocated;
}

int heap_validate(void){
    if (memory_manager.memory_start == NULL || memory_manager.memory_size == 0)return 2;
    chunk_t* memory_chunk = memory_manager.first_memory_chunk;
    while (memory_chunk != NULL){
        if (get_validation_value(memory_chunk) != memory_chunk->validation_value)return 3;
        char* fence_start = (char*)memory_chunk + sizeof(chunk_t);
        for (int i = 0; i < FENCE_SIZE; i++)
            if (fence_start[i] != FENCE || fence_start[i + memory_chunk->size + FENCE_SIZE] != FENCE)
                return 1;
        memory_chunk = memory_chunk->next;
    }
    return 0;
}
