#ifndef H_MANAGER
#define H_MANAGER
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
/***********************************************
* Defines the struct and operations of a manager
* Author: Gloire Rubambiza
* Version: 09/26/2017
***********************************************/

typedef char* ServerName;

typedef struct Servers {
  ServerName name;
  pid_t server_pid;
  int active_processes;
  int max_process;
} Server;

/**
* Check the minimum requirements on active processes
*/
void check_min ( char * server_args[5], int limits[]);

/**************************************
* Fills  the struct for a given server
**************************************/
void fill_struct (Server* server, const char* name, int limits[]);

/******************************************
* Updates the struct with the child process
*******************************************/
void update_struct (Server* server, pid_t* pid);

/**
 * Create a server and fill the pid
*/
pid_t create_server ( char* tokens[] );

/**
 * Reads in user commands and parses them into tokens
*/
int read_command (char* tokens[]);

/**
 * Displays a prompt for the user to input commands
*/
void display_prompt();

/**
 * Displays the status of the system
 */
void display_status();

/**
 * Parses commands into arguments
*/
int parse_command (char* command_buffer, char* tokens[]);

/**
 * Gracefully shuts down a server and its children
 */
void abort_server ( char* name, Server manager[]);

/**
 * Searches for the server's pid before sending a kill signal
 */
pid_t search_server ( const char* name, Server manager[] );

/**
 * Creates a new process on the given server name 
 */
pid_t create_process ( const char* name , Server manager[]);

/**
 * Aborts one copy of the given server name 
 */
void abort_process ( char* name, Server manager[]);

#endif
