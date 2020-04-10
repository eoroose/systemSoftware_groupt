/*
 * interpretation of my CONNMGR code is derived from "IBM Knowledge Center" website, interpreting from an article called "using poll() instead of select()" 
 * URL: https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_74/rzab6/poll.htm
 */

#ifndef __CONNMGR_H__
#define __CONNMGR_H__

#ifndef TIMEOUT
  #error TIMEOUT not specified!(in seconds)
#endif

void connmgr_listen_buffer(int port_number, void(*write_to_buffer)(sensor_data_t*), void(*write_to_fifo)(char*));
/*
This method holds the core functionality of your connmgr.
It starts listening on the given port and when when a sensor node connects it writes the data to a buffer.
all log messages should go through a fifo pipe
*/

void connmgr_free(void);
/*
This method should be called to clean up the connmgr, and to free all used memory.
After this no new connections will be accepted
*/

#endif