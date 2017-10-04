#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#define MAX_REPLICAS 10
#include "server.h"
#include <time.h>

/*****************************************************
* Server that replicates itself at time of creation
* Author: Gloire Rubambiza
* Version: 09/28/2017
******************************************************/

// Declare all the children for ease of sending them signals
Children child_pids[MAX_REPLICAS];

/**
 * Sends signals to children to recuperate system 
 * resources before terminating 
 * @param child_pids the array of its children's IDs
 */
void terminate_children( Children * child_pids[]){
}

/**
 * General signal handler to handle SIGUSR1, SIGUSR2, SIGINT signals.
 * @param sigNum is the received signal
 */
void server_sig_handler (int sigNum) {
    if (sigNum == SIGUSR1) {
        printf("We got a SIGUSR1!\n");
    }
    if (sigNum == SIGUSR2) {
        printf("We got a SIGUSR2!\n" );
    }
    if (sigNum == SIGINT) { // Gracefully exit when SIGINT received
        pid_t process_pid = getpid();
        printf ("Received an interrupt signal from server manager.\n");
        printf("My process id is %d, I am shutting down\n\n", process_pid);
	printf ("Sending kill signals to all my children....\n\n");
	int i = 0;
        for (i = 0; i < MAX_REPLICAS; ++i){
          pid_t current_child = child_pids[i].child_pid;
          //deallocate_child(&child_pids[i]);
          kill(current_child, SIGINT);
	  waitpid(current_child, NULL, 0);
        }
        exit(0);
    }
}

void replica_sig_handler (int sigNum) {
  if (sigNum == SIGINT){
   printf("Child process here, shutting down...\n\n");
   exit(0);
  }
}
/**
 * Allocates memory for all the children's pids
 * To be used when replicating children
 * @param child_pids the array of its children's IDs
 */
void allocate_child( Children * child){
   child = malloc(sizeof(Children));
   child->taken = true;
}

/**
 * Frees memory for the struct of a child 
 * To be used when shutting down a server 
 * @param child_pids the array of its children's IDs
 */
void deallocate_child( Children * child){
   free(child);
}

/**
 * Replicates the server a given number of times 
 * @param child_pids the array of its children's pids
 * @return 0 if children were successfuly created,
 * otherwise we return -1
 */
int replicate ( int num, pid_t* parent_pid, Children child_pids[]) {
  
  // Loop through the children and create a replica as necessary 
  int child;
  for (child = 0; child < num; ++child){
    
    pid_t pid, temp_pid1, temp_pid = getpid();
    if ( temp_pid == *parent_pid) { // Only the parent is allowed to fork
      pid = fork();
      temp_pid1 = pid;
      if ( pid < 0){
        fprintf(stderr, "Forking child #%d failed!!\n", child);
        return -1;
      }
    } 
    if ( pid > 0 ) { // The parent updates this child's struct
      allocate_child(&child_pids[child]);
      child_pids[child].child_pid = temp_pid1;
    }
    if ( pid == 0 ) {
      int my_pid = getpid();
      printf("[Replica]: I am child #%d, with pid %d, of parent %d\n\n", 
	   child, my_pid, *parent_pid);
      printf ("I'm gonna just infinetely execute some no-ops over here!\n");
      
      // Register kill signal from parent server
      signal(SIGINT, replica_sig_handler);
      
      // Execute infitely
      while(true);
      //exit(0);
    }
      
  }
  return 0;
}

int main(int argc, char* argv[]){
  
  printf("[%s]: I am a newly created server, spawning %d "
          "children soon\n\n", argv[1], atoi(argv[2]));
  
  // Global variables for the children pids and arguments passed in
  char* my_sname = argv[2];
  int fork_success, num_active = atoi(argv[2]);
  pid_t parent_pid = getpid();
	
  

  // Fork the children num_active number of times
  fork_success = replicate(num_active, &parent_pid, child_pids);
  if ( (fork_success) < 0){
    fprintf(stderr, "There was an error replicating child "
               "processes in the %s server\n", my_sname);
  }
  
  //sleep(30);
  int j;
  for (j =0; j < num_active; ++j){
    printf("Child # %d is %d, pid is %d\n\n",
	   j,child_pids[j].taken,child_pids[j].child_pid);
  }

  // Register the CTRL-C signal from the manager
  signal(SIGINT, server_sig_handler); 
  pause();
  exit(0);
}
