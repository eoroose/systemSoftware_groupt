#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#include "config.h"

#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS  0
#define SBUFFER_NO_DATA  1

typedef struct sbuffer sbuffer_t;

/*
 * Allocates and initializes a new shared buffer
 * Returns SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
 */
int sbuffer_init(sbuffer_t ** buffer);

/*
 * All allocated resources are freed and cleaned up
 * Returns SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
 */
int sbuffer_free(sbuffer_t ** buffer);

/*
 * Removes the first sensor data in 'buffer' (at the 'head') and returns this sensor data as '*data'
 *    REMARK: removes the head permenantly  ONLY when all the "READERS" have requested the head through their "id"
 *    a specific reader does not have to wait for any other reader to get new data, it only has to wait for the writer to send new data
 *      readers are not dependant on each others speed, only on the writers speed to add new data
 * 
 * 'data' must point to allocated memory because this functions doesn't allocated memory
 * If 'buffer' is empty, the function doesn't block until new sensor data becomes available but returns SBUFFER_NO_DATA
 * Returns SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
 */
int sbuffer_remove(sbuffer_t * buffer,sensor_data_t * data, int id);

/* Inserts the sensor data in 'data' at the end of 'buffer' (at the 'tail')
 * Returns SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
*/
int sbuffer_insert(sbuffer_t * buffer, sensor_data_t * data);

#endif  //_SBUFFER_H_

