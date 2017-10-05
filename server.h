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
void server_sig_handler (int sigNum);

/*
 *Shuts down a server's child process
  @param sigNum the signal sent to the child
*/
void replica_sig_handler (int sigNum);


/**
 * Replicates the server a given number of times 
 */
int replicate ( int num, pid_t* parent_pid, Children child_pids[]);

#endif
