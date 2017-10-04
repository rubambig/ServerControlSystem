#ifndef H_SERVER
#define H_SERVER
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>


/***********************************************
* Defines the struct and operations of a server
* Author: Gloire Rubambiza
* Version: 09/26/2017
***********************************************/

typedef bool available;

// The structure to keep track of a server's children
typedef struct Children {
    available taken;
    pid_t child_pid;
} Children;

/**
 * Allocates memory for all the children's pids
 * @param child_pids the array of its children's IDs
 */
void allocate_child( Children * child);

/**
 * Frees memory for the struct of a child 
*/
void deallocate_child( Children * child);

/**
 * General signal handler to handle SIGUSR1, SIGUSR2, SIGINT signals.
 * @param sigNum is the received signal
 */
void sig_terminate_handler (int sigNum);
  
/**
 * Sends signals to children to recuperate system 
 * resources before terminating 
 * @param child_pids the array of its children's IDs
 */
void terminate_children( Children * child_pids[]);

/**
 * Replicates the server a given number of times 
 */
int replicate ( int num, pid_t* parent_pid, Children child_pids[]);

#endif
