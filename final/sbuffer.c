#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "sbuffer.h"

typedef struct sbuffer_node {
  struct sbuffer_node * next;
  sensor_data_t data;
  int readers[READERS];
} sbuffer_node_t;

struct sbuffer {
  sbuffer_node_t * head;
  sbuffer_node_t * tail;
};

int sbuffer_init(sbuffer_t ** buffer) {
  *buffer = malloc(sizeof(sbuffer_t));
  if (*buffer == NULL) return SBUFFER_FAILURE;
  (*buffer)->head = NULL;
  (*buffer)->tail = NULL;
  return SBUFFER_SUCCESS; 
}

int sbuffer_free(sbuffer_t ** buffer) {
  sbuffer_node_t * dummy;
  if ((buffer==NULL) || (*buffer==NULL)) {
    return SBUFFER_FAILURE;
  } 
  while ( (*buffer)->head ) {
    dummy = (*buffer)->head;
    (*buffer)->head = (*buffer)->head->next;
    free(dummy);
  }
  free(*buffer);
  *buffer = NULL;
  return SBUFFER_SUCCESS;		
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t * data, int id) {
  sbuffer_node_t *dummy;
  if(id >= READERS || buffer == NULL) return SBUFFER_FAILURE;
  if(buffer->head == NULL) return SBUFFER_NO_DATA;
  dummy = buffer->head;
  
  while(dummy != NULL) {
    if(dummy->readers[id] == 1) dummy = dummy->next;
    else break;
  }

  if(dummy == NULL) return SBUFFER_NO_DATA;
  else *data = dummy->data;
  dummy->readers[id] = 1;
  
  //check if remove or not
  if (dummy != buffer->head) return SBUFFER_SUCCESS;
  for(int i = 0; i < READERS; i++) if(dummy->readers[i] == 0) return SBUFFER_SUCCESS;


  if (buffer->head == buffer->tail) { // buffer has only one node
    buffer->head = buffer->tail = NULL; 
  }
  else { // buffer has many nodes 
    buffer->head = buffer->head->next;
  }
  free(dummy);
  return SBUFFER_SUCCESS; 
}

int sbuffer_insert(sbuffer_t * buffer, sensor_data_t * data) {
  sbuffer_node_t * dummy;
  if (buffer == NULL) return SBUFFER_FAILURE;
  dummy = malloc(sizeof(sbuffer_node_t));
  if (dummy == NULL) return SBUFFER_FAILURE;
  dummy->data = *data;
  dummy->next = NULL;
  memset(&dummy->readers, 0, sizeof(dummy->readers));

  if (buffer->tail == NULL) { // buffer empty (buffer->head should also be NULL 
    buffer->head = buffer->tail = dummy;
  } 
  else { // buffer not empty 
    buffer->tail->next = dummy;
    buffer->tail = buffer->tail->next; 
  }
  return SBUFFER_SUCCESS;
}