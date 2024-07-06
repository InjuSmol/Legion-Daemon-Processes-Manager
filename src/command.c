#include "command.h"
#include <unistd.h> // for standard i/o associated with terminal


#include "main_helper.h"
#include "processes.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#define LOGFILE_FORMAT "%s/%s.log.%c"

typedef struct users_input users_input;


// Method to free the user_unput struct 
void free_users_input(users_input *input) {
    if (input == NULL) {
        return; 
    }   
    int size_args = input -> num_args;
    // Set pointers to NULL to avoid dangling pointers
    input->command = NULL;
    input->name = NULL;
    
    // Free dynamically allocated memory for each member
    free(input->command);
    free(input->name);
    /*
    printf("  size: %d\n",array_size(input->arguments) );
  
       
    for (int i = 0; i < size; i++) {
        printf("  %d: %s\n", i, input->arguments[i]);
    }
    */
    //int size = array_size(input->arguments)-1;
    if(input->arguments != NULL) {
       //printf("  size: %d\n",array_size(input->arguments) );
        for (int i = 0; i < size_args; i++) {
            free(input->arguments[i]);
        /*
        if (input->arguments[i] != NULL)
        //printf("  size: %d\n",array_size(input->arguments) );
            free(input->arguments[i]);
        }
        */
        }
        //printf("  size: %d\n",array_size(input->arguments) );
        free(input->arguments);
    }   
    
     
}

// Method to initialize an user input onject: 
users_input *init_users_input() {    
    users_input *input = malloc(sizeof(users_input));
    if (input == NULL) {
        // Memory allocation failed
        perror("Error allocating memory for users_input");
        exit(EXIT_FAILURE);
    }

    // Initialize individual pointers
    input->command = NULL;
    input->name = NULL;
    input->arguments = NULL;
    input -> num_args = 0;

    // Return the initialized struct
    return input;
}


// Legion commands: 

// Status command: 
int status_c(char *name, FILE *out){
    //Daemon *daemon = find_daemon(name);
    //if (daemon == NULL){
    //    return -1; // error TODO: What are we doing on errors? 
    //}
    //sf_status();
    pid_t pid = get_daemon_pid(name); // (long) and print with ld
    char *status = get_status(name);
    if (pid == -2 || status ==NULL){
        //fprintf(out,"%s\n","Error executing command: status");
        return -1; // HANDLE ERROR
    }
    
    //const char* status_str = get_status_str(status);    
    char *msg = (char *)malloc(sizeof(name) + sizeof(pid_t) + sizeof(status));
    
    sprintf(msg, "%s\t%ld\t%s", name, (long)pid, status);
    //fprintf(out, "%s\t%ld\t%s", name, (long)pid, status); 
    sf_status(msg);
    fprintf(out,"%s\n", msg); 
    return 0;
}



int status_all_c(FILE * out){
    for (int i = 0;  i < num_daemons_g; i++){
        if (daemons_g[i].name == NULL){
            return -1; //error, but why? 
        }
        else 
            if (status_c(daemons_g[i].name, out)== -1){
                return -1;
            }
    }
    return 0;
}

// Help command: 
void help_c(FILE *out){
    //HELP(FILE *out);
    fprintf(out, "Available commands: %s\n",
    "quit (0 args) Quit the program\n" 
    "register (0 args) Register a daemon\n" 
    "unregister (1 args) Unregister a daemon\n" 
    "status (1 args) Show the status of a daemon\n" 
    "status-all (0 args) Show the status of all daemons\n" 
    "start (1 args) Start a daemon\n" 
    "stop (1 args) Stop a daemon\n" 
    "logrotate (1 args) Rotate log files for a daemon"); 
}

// Quit command: 
// eqit the program after ensuring that every active daemon is terminated: 
void quit_c(FILE *out){
    // STEP 1: Check if every active daemon is terminated
    
    // STEP 2: if not, terminate all the daemons
    
    // STEP 3: quot, exit the program
}
/*
typedef struct daemon{
   char *name; 
   pid_t pid; // TODO: is it right? 
   char *status;
   char ** args; 
      // TODO: is it right?    
} Daemon;

Daemon *daemons_g; // dynamic array to store our daemons. Will allocate memory dynamically for them 
int num_daemons_g = 0; // global variable to keep track of the daemons created: (increase every time when we call new_daemon and decrease every time when we call delete daemon)
*/
// Method to register a new daemon: 
Daemon *register_c(FILE *out, char *name, char *exe_name, char **args, int num_args){
    //printf("\n%s\n", "REACHED REGISTER_C!"); // TODO: Remove it !!!!!!!!!
    // Initialize a new daemon: 
   
    char* new_status = "inactive"; // TODO: Should the status initially be inactive or unknown? 
    Daemon *new_daemon = create_daemon(out, name, new_status, (pid_t)0, args, num_args); // What is pid in the begining? 
    //print_daemons(); // TODO: remove this !!!!!!!!!!
    return new_daemon; 
}

// Method to unregister a daemon process: 
int unregister_c(FILE *out, char *name){
    Daemon *daemon = find_daemon(name); 
    if (daemon == NULL){
        return -1; 
    } 
    if (strcmp(daemon -> status, "inactive") != 0){
        return -1; // error  
    }
    return delete_daemon( name); // returns -1 on error and 0 on sucssess
    //return 0; // no error
}



// Method for logrotating: 
void logrotate_c(FILE *out, char *daemon_name) {
    // Create the log directory if it doesn't exist
    if (access(LOGFILE_DIR, F_OK) == -1) {
        if (mkdir(LOGFILE_DIR, 0777) == -1) {
            fprintf(out,"%s", "Failed to create log directory");
            return;
        }
    }
    // Check if the daemon is registered:
    Daemon *daemon = find_daemon(daemon_name);
    if (daemon == NULL) {
            fprintf(out, "Daemon '%s' is not registered.\n", daemon_name);
            sf_error("Error executing command");
            fprintf(out, "Error executing command: %s\n", "logrotate");
        return;
    }
    // Rotate log files:
    char current_version = LOG_VERSIONS - 1;
    char old_log_file[PATH_MAX];
    char new_log_file[PATH_MAX];

    // Delete the log file with the maximum version number:
    snprintf(old_log_file, PATH_MAX, LOGFILE_FORMAT, LOGFILE_DIR, daemon_name, current_version);
    if (unlink(old_log_file) == -1 && errno != ENOENT) {
        fprintf(out,"%s", "Failed to delete old log file");
        return;
    }
    // Rename each log file to have a version one greater than it did before:
    for (char version = current_version - 1; version >= '0'; version--) {
        snprintf(old_log_file, PATH_MAX, LOGFILE_FORMAT, LOGFILE_DIR, daemon_name, version);
        snprintf(new_log_file, PATH_MAX, LOGFILE_FORMAT, LOGFILE_DIR, daemon_name, version + 1);

        if (rename(old_log_file, new_log_file) == -1 && errno != ENOENT) {
            perror("Failed to rename log file");
            return;
        }
    }
    // Restart the daemon if it was active:
    if (strcmp(daemon->status, "active") == 0) {
        fprintf(out, "Restarting daemon '%s' after log rotation.\n", daemon_name);
        stop_c(out, NULL, daemon_name); // Stop the daemon
        start_c(out, NULL, daemon_name); // Start the daemon
    }
}




