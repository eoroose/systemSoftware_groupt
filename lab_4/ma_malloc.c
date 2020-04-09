#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "ma_malloc.h"

#define MEM_POOL_SIZE 600   // in bytes
#define HEADER_SIZE 4       // in bytes

typedef unsigned char byte;

typedef enum {ALLOCATED, FREE} mem_status;

typedef struct {
  size size;
  mem_status status;
} mem_chunk_header;

void coaless(void);

static byte mem_pool[MEM_POOL_SIZE];

/*
** Allocates array of bytes (memory pool) and initializes the memory allocator. 
** If some bytes have been used after calling ma_malloc(size), calling to ma_init() will result in clearing up the memory pool.
*/
void ma_init() {
	mem_chunk_header *mem_ptr = (mem_chunk_header*)mem_pool;
    mem_ptr->size = MEM_POOL_SIZE - HEADER_SIZE;
    mem_ptr->status = FREE;
}

/*
** Requesting for the tsize bytes from memory pool. 
** If the request is possible, the pointer to the first possible address byte (right after its header) in memory pool is returned.
*/
void *ma_malloc(size tsize) {
    int current_mem = 0;
	mem_chunk_header *mem_ptr = (mem_chunk_header*)mem_pool;

    while(1) {
        if(mem_ptr->status == FREE && mem_ptr->size >= HEADER_SIZE + tsize) {
            mem_chunk_header *new_mem_ptr = mem_ptr + HEADER_SIZE + tsize;
            new_mem_ptr->size = mem_ptr->size - HEADER_SIZE - tsize;
            new_mem_ptr->status = FREE;

            mem_ptr->size = tsize;
            mem_ptr->status = ALLOCATED;
            return mem_ptr;
        }
        current_mem += HEADER_SIZE + mem_ptr->size;
        if(current_mem >= MEM_POOL_SIZE) break;
        mem_ptr += HEADER_SIZE + mem_ptr->size;
    }
    return NULL;
}

/*
** Releasing the bytes in memory pool which was hold by ptr, meaning makes those bytes available for other uses. 
** Implement also the coalescing behavior.
*/
void ma_free(void* ptr) {
    mem_chunk_header *mem_ptr = (mem_chunk_header*)ptr;
    mem_ptr->status = FREE;
    coaless();
}

/*
** This function is only for debugging. It prints out the entire memory pool. 
** Use the code from the memdump tool to do this.
*/
void ma_print(void) {
    int current_mem = 0;
    mem_chunk_header *mem_ptr = (mem_chunk_header*)mem_pool;
    int chunck_size = mem_ptr->size + HEADER_SIZE;

    while (1) {
        if(mem_ptr->status == FREE) {
            for(int i = 0; i < chunck_size; i++) {
                if(i < HEADER_SIZE) printf("H ");
                else printf (". ");
                
            }
        }
        else if(mem_ptr->status == ALLOCATED) {
            for(int i = 0; i < chunck_size; i++) {
                if(i < HEADER_SIZE) printf("H ");
                else printf ("A ");
            }
        }
        current_mem += chunck_size;
        if(current_mem == MEM_POOL_SIZE) break;

        mem_ptr += chunck_size;
        chunck_size = mem_ptr->size + HEADER_SIZE;
    }
    printf("\n");
}
 
void coaless() {
    mem_chunk_header *current_mem_ptr = (mem_chunk_header*)mem_pool, *next_mem_ptr;
    int current_mem = 0;

    while(current_mem + HEADER_SIZE + current_mem_ptr->size < MEM_POOL_SIZE) {
        next_mem_ptr = current_mem_ptr + HEADER_SIZE + current_mem_ptr->size;

        if(current_mem_ptr->status == FREE && next_mem_ptr->status == FREE) {
            current_mem_ptr->size += HEADER_SIZE + next_mem_ptr->size;
        }
        else {
            current_mem += HEADER_SIZE + current_mem_ptr->size;
            current_mem_ptr = next_mem_ptr;
        }
    }
}