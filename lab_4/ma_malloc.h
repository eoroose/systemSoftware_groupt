#ifndef MA_MALLOC_H_
#define MA_MALLOC_H_

typedef int size;

/*
** Allocates array of bytes (memory pool) and initializes the memory allocator. 
** If some bytes have been used after calling ma_malloc(size), calling to ma_init() will result in clearing up the memory pool.
*/
void ma_init();

/*
** Requesting for the tsize bytes from memory pool. 
** If the request is possible, the pointer to the first possible address byte (right after its header) in memory pool is returned.
*/
void* ma_malloc(size tsize);

/*
** Releasing the bytes in memory pool which was hold by ptr, meaning makes those bytes available for other uses. 
** Implement also the coalescing behavior.
*/
void ma_free(void* ptr);

/*
** This function is only for debugging. It prints out the entire memory pool. 
** Use the code from the memdump tool to do this.
*/
void ma_print(void);


#endif //MA_MALLOC_H_ 