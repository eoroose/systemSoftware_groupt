#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR      0
#define DPLIST_MEMORY_ERROR  1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
	#define DEBUG_PRINTF(...) 									                                    \
		do {											                                            \
			fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	    \
			fprintf(stderr,__VA_ARGS__);								                            \
			fflush(stderr);                                                                         \
                } while(0)
#else
	#define DEBUG_PRINTF(...) (void)0
#endif

#define DPLIST_ERR_HANDLER(condition,err_code)                      \
	do {						                                    \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");  \
            assert(!(condition));                                   \
        } while(0)

struct dplist_node {
    dplist_node_t * prev, * next;
    void * element;
};

struct dplist {
    dplist_node_t * head;
    void * (*element_copy)(void * src_element);			  
    void (*element_free)(void ** element);
    int (*element_compare)(void * x, void * y);
};

dplist_node_t * dpl_get_reference_in_list( dplist_t *, dplist_node_t *);

dplist_t * dpl_create ( void * (*element_copy)(void * element),
			            void (*element_free)(void ** element),	
			            int (*element_compare)(void * x, void * y)
			          ) {
    DEBUG_PRINTF("hello\n");
    dplist_t * list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;  
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;        
    }

void dpl_free(dplist_t ** list, bool free_element) {
    DPLIST_ERR_HANDLER(*list == NULL, DPLIST_INVALID_ERROR);
    
    while((*list)->head != NULL) {
        if(free_element == true) (*list)->element_free(&((*list)->head->element));
        
        if((*list)->head->next != NULL) {
            (*list)->head = (*list)->head->next;
            free((*list)->head->prev);
            (*list)->head->prev = NULL;
        } else {
            free((*list)->head);
            (*list)->head = NULL;
        }   
    }
    free(*list);
    *list = NULL;
}

dplist_t * dpl_insert_at_index(dplist_t * list, void * element, int index, bool insert_copy) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    int size = dpl_size(list);
    
    dplist_node_t *new_node = malloc(sizeof(dplist_node_t));
    
    DPLIST_ERR_HANDLER(new_node == NULL, DPLIST_MEMORY_ERROR);
    new_node->element = insert_copy == true ? list->element_copy(element) : element;
    
    dplist_node_t *node_at_index = dpl_get_reference_at_index(list, index);
    
    if(size == 0) {                             //fisrt element
        new_node->next = NULL;
        new_node->prev = NULL;
        list->head = new_node;
    } else if(index <= 0) {                     //head
        new_node->next = node_at_index;
        new_node->prev = NULL;
        node_at_index->prev = new_node;
        list->head = new_node;
    } else if(index >= size) {                  //tail
        new_node->next = NULL;
        new_node->prev = node_at_index;
        node_at_index->next = new_node;
    } else {                                    //any in between
        new_node->next = node_at_index;
        new_node->prev = node_at_index->prev;
        node_at_index->prev->next = new_node;
        node_at_index->prev = new_node;
    }
    
    return list;
}

dplist_t * dpl_remove_at_index( dplist_t * list, int index, bool free_element) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    if(list->head == NULL) return list;

    dplist_node_t *node_at_index = dpl_get_reference_at_index(list, index);
    if(node_at_index == list->head) list->head = node_at_index->next;
    if(free_element == true) list->element_free(&(node_at_index->element));
    
    if(node_at_index->prev == NULL && node_at_index->next != NULL) {        //head
        node_at_index->next->prev = NULL;
    }
    else if(node_at_index->prev != NULL && node_at_index->next == NULL) {   //tail
        node_at_index->prev->next = NULL;
    }
    else if(node_at_index->prev != NULL && node_at_index->next != NULL) {   //any in between
        node_at_index->prev->next = node_at_index->next;
        node_at_index->next->prev = node_at_index->prev;
    }
    free(node_at_index);
    node_at_index = NULL;

    return list;
}

int dpl_size( dplist_t * list ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    int count = 0;
    dplist_node_t *node = list->head;
    while(node != NULL) {
        count++;
        node = node->next;
    }
    return count;
}

dplist_node_t * dpl_get_reference_at_index( dplist_t * list, int index ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    int size = dpl_size(list);
    dplist_node_t *node = list->head;
    for(int current = 0; node != NULL; node = node->next, current++) {
        if(current >= index || current == size-1) return node; 
    }
    return NULL;
}

void * dpl_get_element_at_index( dplist_t * list, int index ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *node_at_index = dpl_get_reference_at_index(list, index);
    if(node_at_index == NULL) return (void*)0;
    return node_at_index->element;
}

int dpl_get_index_of_element( dplist_t * list, void * element ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *node = list->head;
    int index;
    if(list->head == NULL) return -1;
    for(index = 0; list->element_compare(node->element, element) != 0; node = node->next, index++) {
        if(node->next == NULL) return -1;
    }
    return index;
}

// HERE STARTS THE EXTRA SET OF OPERATORS //

// ---- list navigation operators ----//
  
dplist_node_t * dpl_get_first_reference( dplist_t * list ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    return list->head;
}

dplist_node_t * dpl_get_last_reference( dplist_t * list ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    return dpl_get_reference_at_index(list, dpl_size(list));
}

dplist_node_t * dpl_get_next_reference( dplist_t * list, dplist_node_t * reference ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *node = dpl_get_reference_in_list(list, reference);
    if(node == NULL) return NULL;
    return node->next;
}

dplist_node_t * dpl_get_previous_reference( dplist_t * list, dplist_node_t * reference ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *node = dpl_get_reference_in_list(list, reference);
    if(node == NULL) return NULL;
    return node->prev;
}

dplist_node_t * dpl_get_reference_in_list( dplist_t * list, dplist_node_t * reference) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    if(reference == NULL) return NULL;
    dplist_node_t *node;
    for(node = list->head; node != NULL; node = node->next) {
        if(node == reference) return node;
    }
    return NULL;
}
// ---- search & find operators ----//  
  
void * dpl_get_element_at_reference( dplist_t * list, dplist_node_t * reference ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    if(reference == NULL) reference = dpl_get_last_reference(list);
    if(dpl_get_reference_in_list(list, reference) == NULL) return NULL;
    return reference->element;
}

dplist_node_t * dpl_get_reference_of_element( dplist_t * list, void * element ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *node;
    for(node = list->head; node != NULL; node = node->next) {
        if(list->element_compare(node->element, element) == 0) return node;
    }
    return NULL;
}

int dpl_get_index_of_reference( dplist_t * list, dplist_node_t * reference ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    if(reference == NULL) reference = dpl_get_last_reference(list);
    dplist_node_t *node;
    int index;
    for(index = 0, node = list->head; node != NULL; node = node->next, index++) {
        if(node == reference) return index;
    }
    return -1;
}
  
// ---- extra insert & remove operators ----//

dplist_t * dpl_insert_at_reference( dplist_t * list, void * element, dplist_node_t * reference, bool insert_copy ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    if(reference == NULL) reference = dpl_get_last_reference(list);
    int index = dpl_get_index_of_reference(list, reference);
    if(index == -1) return list;
    return dpl_insert_at_index(list, element, index, insert_copy);
}

dplist_t * dpl_insert_sorted( dplist_t * list, void * element, bool insert_copy ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    dplist_node_t *node;
    int index;
    for(index = 0, node = list->head; node != NULL; node = node->next, index++) {
        if(list->element_compare(element, node->element) <= 0) break;
    }
    return dpl_insert_at_index(list, element, index, insert_copy);
}

dplist_t * dpl_remove_at_reference( dplist_t * list, dplist_node_t * reference, bool free_element ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    if(reference == NULL) reference = dpl_get_last_reference(list);
    int index = dpl_get_index_of_reference(list, reference);
    if(index == -1) return list;
    return dpl_remove_at_index(list, index, free_element);
}

dplist_t * dpl_remove_element( dplist_t * list, void * element, bool free_element ) {
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    int index = dpl_get_index_of_element(list, element);
    if(index == -1) return list;
    return dpl_remove_at_index(list, index, free_element);
}

