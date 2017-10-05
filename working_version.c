#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "manager.h"
#define MAX_SERVERS 10
#define MIN_REPLICAS 2
#define STR_BUFFER_SIZE 255 // A linux file cannot be >255 characters long
#define MAX_ARGS 6

/*****************************************************
* Main server manager that creates all servers
* Manages all structs associated with server instances
* Author: Gloire Rubambiza
* Version: 09/28/2017
******************************************************/

/*****************************************************
Fills the server's struct for housekeeping purposes
* Server manager uses this to keep track of the server
* @param server the struct to be filled
* @param parent_pid the server's manager aka its parent
* @param arguments the parameters passed by the user
******************************************************/
void fill_struct(Server* server, const char* name, int limits[]){
  server->name = malloc(sizeof(ServerName));
  strcpy(server->name, name);
  server->active_processes = limits[0];
  server->max_process = limits[1];
}

/**
* Updates all server structs with newly created pids
* @param server the struct to be updated
*/
void update_struct (Server* server, pid_t* pid){
  server->server_pid = *pid;
}

/**
* Sends the server to execute in a different process
@param server the server to be created
@return 0 on successful creation of server, otherwise error
*/
pid_t create_server ( char* tokens[]){
 
  pid_t pid = fork();
  if ( pid == 0){
    execvp(tokens[0], tokens);
  }
  if ( pid == -1){
    printf("There was an error forking\n\n");
  }
  return pid;
}

/**
 * Displays a prompt for the user to input commands
*/
void display_prompt(){
  printf("Please enter the next command --> ");
}

/**
 * Displays the current status of the system
 */
void display_status(){
  
  int status;
  pid_t pid = fork();
  if (pid == 0){
    char* command[] = {"/bin/bash", "-c", "ps f", NULL};
    execvp(command[0], command);
  } else {
    wait(&status);
  }
}
/**
 * Reads and parses the command entered by the user, and stores it in
 * pre-allocated buffers.
 * @param tokens is the pointer to the array that holds the user's tokenized
 *        command.
 * @return -1 if there's an error, 1 if no command was entered, 0 otherwise.
 */
int read_command(char* tokens[]) {
    int max_size = STR_BUFFER_SIZE * MAX_ARGS;
    char* command_buffer = malloc(max_size);
    if (fgets(command_buffer, max_size, stdin) == NULL) {
        fprintf(stderr, "There was an error reading user input.\n");
        return -1;
    }
    int parse_result = parse_command(command_buffer, tokens);
    if (parse_result < 0) {
        fprintf(stderr, "Could not parse the command.\n");
    }
    return parse_result;
}

/**
 * Parses the user input into an array of string tokens that are separated by
 * tab, newline or space characters.
 * @param command_buffer is the user input.
 * @param tokens is the array that will hold the parsed tokens.
 * @return -1 if an error occurs, 1 if no command was entered, 0 otherwise.
 */
int parse_command(char* command_buffer, char* tokens[]) {
    int count = 1;
    char* delimiter = "\t \n";
    char* current_token = strtok(command_buffer, delimiter);
    if (current_token == NULL) {
        //resets tokens array to empty strings
        int i;
        for (i = 1; i < MAX_ARGS; ++i) {
            tokens[i] = NULL;
        }
        return 1;
    }
    while(current_token != NULL) {
        tokens[count] = current_token;
        current_token = strtok(NULL, delimiter);
        count++;
        if (count >= MAX_ARGS) {
            fprintf(stderr, "Too many arguments given!\n");
            return -1;
        }
    }
    free(command_buffer);
    return 0;
}

/**
 * Searches for the server's pid before sending a kill signal.
 */
pid_t search_server ( const char* name, Server manager[] ){
  int k;
  pid_t ts_pid;
  for ( k = 0; k < MAX_SERVERS; ++k) {
    if ( manager[k].name != NULL) { // Avoids comparing to null pointers.
        if( strcmp(manager[k].name,name) == 0){
          ts_pid = manager[k].server_pid;
	  free(manager[k].name);
          return ts_pid ;
      }
    }
  }
  return -1;
}

/**
 * Creates a new process on the given server name 
 * @param name is the name of the server
 * @param manager the server manager
 * @return pid if we are not exceeding the limits 
 * 0 if we are passing the limit 
 * -1 if the given server does not exist
 */
pid_t create_process ( const char* name, Server manager[]){
  int k;
  pid_t ts_pid;
  for ( k = 0; k < MAX_SERVERS; ++k) {
    if ( manager[k].name != NULL ) { // Avoids comparing to null pointers.
	int max = manager[k].max_process;
	int active = manager[k].active_processes;
        if( strcmp(manager[k].name,name) == 0){
	  if ( active + 1 <= max){
	    ts_pid = manager[k].server_pid;
	    manager[k].active_processes++;
            return ts_pid ;
	  } else {
	    return 0;
	  }
      }
    }
  }
  return -1;
}
int main(int argc, char* argv[]){
  
  if (argc < 1){
    printf("Please enter ./executable createServer min max\n\n");
    exit(0);
  }

  printf("[Server Manager]: Started server manager\n" );
  
  // Global variables for server and process limits
  Server manager[MAX_SERVERS];
  int server_count = 0;
  int proc_limits[2];
  pid_t pid;
  const char* name;

  // Arguments to be passed to child processes via vector pointer
  char* server_args[MAX_ARGS];
  int i;
  for ( i = 0; i<4; ++i){
    server_args[i] = malloc(sizeof(char));
  }
  server_args[0] = "./server.o";
  
  // Keep waiting for user input for the next command
     while (1) {
       
       display_prompt();
       if (read_command(server_args) != 0) {
           continue;
       }
       
       if ( strcmp (server_args[1], "createServer") == 0){
	 
	 // Assign arguments for the struct of the given server.
	 name = server_args[2];
	 proc_limits[0] = atoi(server_args[3]);
	 proc_limits[1] = atoi(server_args[4]);
	 
	 // Create the server and update its struct
	 fill_struct(&manager[server_count], name, proc_limits);
	 pid = create_server(server_args);
	 update_struct(&manager[server_count],&pid);
	 server_count++;
	   
       } else if ( strcmp(server_args[1], "abortServer") == 0){
         name = server_args[2];
 
	 // Search for server to send a kill signal.
	 int target_server_pid = (search_server(name, manager)); 
	 if ( target_server_pid < 0){
	   fprintf(stderr, "ERROR: no server found under name %s\n", name);
	 } else { // Send the signal for the server to shut down.
           
	   // Decrement the number of servers in the pool.
           server_count--;
	   int status;
	   kill(target_server_pid, SIGUSR1);
	   wait(&status);

	 }
       } else if ( strcmp(server_args[1], "quit") == 0){
	 exit(0);
       } else if ( strcmp(server_args[1], "displayStatus") == 0){
	 display_status();
       }
       else if ( strcmp(server_args[1], "createProcess") == 0){
         name = server_args[2];
	 
	 // Search for the server that will create a process 
	 int target_server_pid = (create_process(name, manager)); 
	 if ( target_server_pid < 0){
	   fprintf(stderr, "ERROR: no server found under name %s\n", name);
	 } else if (target_server_pid > 0)  { // Send the signal for the server to shut down.
           
	   int status;
	   kill(target_server_pid, SIGUSR2);
	   wait(&status);

	 } else {
	   printf("Sorry, server %s is at full capacity\n", name);
	 }
       } else if ( strcmp(server_args[1], "abortProcess") == 0){
         // Do stuff for abort process
       } 
	
     }
}

