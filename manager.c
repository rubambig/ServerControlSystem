#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "manager.h"
#define MAX_SERVERS 10
#define MIN_REPLICAS 2
#define STR_BUFFER_SIZE 255 // A linux file cannot be >255 characters long
#define MAX_ARGS 5

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
  printf("My server name is %s\n", server->name);
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
*/
void create_server ( char* tokens[] ){
  int status = execvp(tokens[4], tokens);
  if ( status < 0) {
    printf("The return status of child exec is %d\n", status);
    fprintf(stderr, "Error: Could not start server.\n");
    exit(-1);
  };
}

/**
 * Displays a prompt for the user to input commands
*/
void display_prompt(){
  printf("Please enter the next command --> ");
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
    free(command_buffer);
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
    int count = 0;
    char* delimiter = "\t \n";
    char* current_token = strtok(command_buffer, delimiter);
    if (current_token == NULL) {
        //resets tokens array to empty strings
        int i;
        for (i = 0; i < MAX_ARGS; ++i) {
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
int main(int argc, char* argv[]){
  pid_t parent_id = getpid();
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
  
  strcpy(server_args[1], argv[2]);
  strcpy(server_args[2], argv[3]);
  strcpy(server_args[3], argv[4]);
  server_args[4] = "./server.o";
  
  printf("[Server Manager]: Started manager, its pid is %d\n", parent_id);

  if ( strcmp(argv[1],"createServer") == 0){

    // Check that the min_process passed in is not less than the default
    if ( (atoi(argv[3])) < MIN_REPLICAS) {
      printf("[Server Manager]: Minimum processes set to default of 2\n");
      proc_limits[0] = MIN_REPLICAS;
    } else {
      proc_limits[0] = atoi(argv[3]);
    }
    
    // Fill the globals before passing them to fill struct
    name = argv[2];
    proc_limits[1] = atoi(argv[4]);
    fill_struct(&manager[server_count], name, proc_limits);
  }
  
  if ( ( pid = fork()) < 0){
    fprintf(stderr, "Forking the server failed!\n");
  }

  /*if ( pid > 0 ) { // The parent keeps executing from here
      // Update the child's struct
      update_struct(&manager[server_count], &pid);
      server_count++;

     // Keep waiting for user input for the next command
     while (1) {
       //char* tokens[MAX_ARGS];
       display_prompt();
       if (read_command(server_args) != 0) {
           continue;
       }
       // Check for all possible operations
       if ( strcmp(server_args[0], "createServer") == 0){
         
	 // Assign arguments for the struct of the given server
	 const char* temp_name = server_args[1];
	 
	 // Check that the min_process passed in is not less than the default
         if ( (atoi(server_args[2])) < MIN_REPLICAS) {
           printf("[Server Manager]: Minimum processes set to default of 2\n");
           proc_limits[0] = MIN_REPLICAS;
         } else {
           proc_limits[0] = atoi(server_args[2]);
         }
         proc_limits[1] = atoi(server_args[3]);
	 fill_struct(&manager[server_count], temp_name, proc_limits);
	 
	 if ( ( pid = fork()) < 0){ // manager forks more servers
           fprintf(stderr, "Forking the server failed!\n");
         }
         
         if ( pid == 0) { // The child starts the server in here
           create_server(server_args);
         }
	 
       } else if ( strcmp(server_args[0], "abortServer") == 0){
	   // Check that the min_process passed in is not less than the default.
           name = server_args[1];
 
	   // Search for server to send a kill signal.
	   pid_t target_server_pid = (search_server(name, manager)); 
	   if ( target_server_pid < 0){
	     fprintf(stderr, "ERROR: no server found under name %s\n", name);
	   } else { // Send the signal for the server to shut down.
           
	   // Decrement the number of servers in the pool.
	   server_count--;
	   
	   kill(target_server_pid, SIGINT);
	   waitpid(target_server_pid, NULL, 0);
           
	 }
       } else if ( strcmp(server_args[0], "createProcess") == 0){
         // Do stuff for createProcess
       } else if ( strcmp(server_args0], "abortProcess") == 0){
         // Do stuff for abort process
       } else if ( strcmp(tserver_args[0], "displayStatus") == 0 ){ 
         // Do stuff for display status
       } else if ( strcmp(server_args[0], "quit") == 0){
	 exit(0);
       }
       
	
     }
   }*/
  if ( pid == 0) { // The child starts the server in here
      create_server(server_args);
  }
  int status;
  wait(&status);
  printf("Child exited with code %d\n\n", status);
  //sleep(15);
  exit(0);



}

