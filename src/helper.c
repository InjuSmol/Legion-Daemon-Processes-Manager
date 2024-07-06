#include "helper.h"

typedef struct daemon Daemon;

 
Daemon *daemons_g; // dynamic array to store our daemons. Will allocate memory dynamically for them 
int num_daemons_g = 0; // global variable to keep track of the daemons created: (increase every time when we call new_daemon and decrease every time when we call delete daemon)

// Method to print the list of tokens: 
void print_tokens(char **list_of_tokens, int num_tokens) {
    if (list_of_tokens == NULL) {
        printf("List of tokens is NULL\n");
        return;
    }


    printf("Tokens:\n");
    for (int i = 0; i < num_tokens; i++) {
        printf("%s\n", list_of_tokens[i]);
    }
}

// Method to find the right daemon from the daemon name provided: 
Daemon *find_daemon(char *name){   
    for (int i = 0; i < num_daemons_g; i++) {
        if (strcmp(daemons_g[i].name, name) == 0) {
            return &daemons_g[i]; // Return a pointer to the found daemon
        }
    }
    return NULL; // If daemon is not found
}

// Method to get daemon status: 
char *get_status(char *name){
    Daemon *daemon = find_daemon(name);
    if (daemon == NULL){
        return NULL;
    }
    return daemon -> status;
}

// Method to get daemons pid: 
pid_t get_daemon_pid(char *name){ 
    Daemon *daemon = find_daemon(name);  
    if (daemon == NULL){
        return -2; // TODO: What return on error here? 
    }
    return daemon -> pid; 
}

// Method to set status of a daemon:
void set_status(FILE *out, Daemon *daemon, char *new_status){
    // free previous status: 
    free(daemon -> status);
    
    daemon -> status = strdup(new_status);
    if (daemon -> status == NULL) {
        fprintf(out, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

}

/*
// Method to create new daemon --> returns a pointer to a new daemon: 
Daemon *create_daemon(FILE *out, char *name, char *status, pid_t pid, char **args) {
    // Check if a daemon with this name already exists: 
    if (find_daemon(name) != NULL){   
        fprintf(out, "%s","Daemon name is already registered.\n");  
        return NULL;
    }
    // Allocate memory for a new daemon
    Daemon *new_daemon = malloc(sizeof(Daemon));
    if (new_daemon == NULL) {
        fprintf(stdout, "Memory allocation failed\n"); // TODO: change the output to the out FILE !!!!
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the name
    new_daemon->name = strdup(name);
    if (new_daemon->name == NULL) {
        fprintf(stdout, "Memory allocation failed\n"); // TODO: change the output to the out FILE !!!!
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the status
    new_daemon -> status = strdup(status);
    if (new_daemon->status == NULL) {
        fprintf(stdout, "Memory allocation failed\n"); // TODO: change the output to the out FILE !!!!
        exit(EXIT_FAILURE);
    }
   
    // Set pid
    new_daemon -> pid = pid;

    // Allocate memory for args and copy the array of strings
    if (args != NULL) {
        //print_arguments(args, array_size(args));
        // get number of arguemnts: 
        int num_args = array_size(args)-1; // TODO: questionable moment
        //while (args[num_args] != NULL) {
        //    num_args++;
        //}
        
        fprintf(stdout, "here x6: %d\n", num_args); // TODO: change the output to the out FILE !!!!
        new_daemon->args = malloc((num_args) * sizeof(char *));
        if (new_daemon->args == NULL) {
            fprintf(stdout, "Memory allocation failed\n"); // TODO: change the output to the out FILE !!!!
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < num_args; i++) {
            new_daemon->args[i] = strdup(args[i]);
            if (new_daemon->args[i] == NULL) {
                fprintf(stdout, "Memory allocation failed\n"); // TODO: change the output to the out FILE !!!!
                exit(EXIT_FAILURE);
            }
        }
        new_daemon->args[num_args] = NULL; // Null-terminate the array of strings
    } else {
        new_daemon->args = NULL;
    }

    // Reallocate memory for the daemons array to include the new daemon
    Daemon *temp = realloc(daemons_g, (num_daemons_g + 1) * sizeof(Daemon));
    if (temp == NULL) {
        fprintf(stdout, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    daemons_g = temp;

    // Add the new daemon to the array
    daemons_g[num_daemons_g++] = *new_daemon; // Note: Here we're copying the struct, not the pointer
    fprintf(stdout, "normal return!!!!\n");
    return new_daemon;
}
*/

// Method to delete daemon --> free memory and delete pointer to it from array daemons 
/*
int delete_daemon(char *name) {
    // Find the daemon with the given name
    for (size_t i = 0; i < num_daemons_g; i++) {
        if (strcmp(daemons[i].name, name) == 0) {
            // Free memory allocated for the daemon's name
            free(daemons[i].name);
            daemons[i].name = NULL;

           

            // Shift remaining daemons to fill the gap
            for (size_t j = i; j < num_daemons_g - 1; j++) {
                daemons[j] = daemons[j + 1];
            }
            num_daemons_g--;

            // Reallocate memory for the daemons array
            daemon *temp = realloc(daemons, num_daemons_g * sizeof(daemon));
            if (temp == NULL && num_daemons_g > 0) {
                fprintf(stderr, "Memory reallocation failed\n");
                exit(EXIT_FAILURE);
            }
            daemons = temp;

            return 1; // Daemon deleted successfully
        }
    }

    // daemon not found
    return 0;
} */

Daemon *create_daemon(FILE *out, char *name, char *status, pid_t pid, char **args, int num_args) {
    // Check if a daemon with this name already exists
    if (find_daemon(name) != NULL) {
        fprintf(out, "Daemon name is already registered.\n");
        return NULL;
    }


    // Allocate memory for a new daemon
    Daemon *new_daemon = malloc(sizeof(Daemon));
    if (new_daemon == NULL) {
        fprintf(out, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    new_daemon -> num_args = num_args;

    // Allocate memory for the name
    new_daemon -> name = strdup(name);
    if (new_daemon->name == NULL) {
        fprintf(out, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the status
    new_daemon->status = strdup(status);
    if (new_daemon->status == NULL) {
        fprintf(out, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Set pid
    new_daemon->pid = pid;

    // Count the number of arguments
    
    if (num_args != 0 && args != NULL) {
    
 
        // Allocate memory for args and copy the array of strings
        new_daemon -> args = malloc((num_args) * sizeof(char *));
        if (new_daemon -> args == NULL) {
            fprintf(out, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num_args; i++) {
            new_daemon->args[i] = strdup(args[i]);
            if (new_daemon->args[i] == NULL) {
                fprintf(out, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
        }
        
    }else if (num_args == 1){ // TODO: Doesnt make sense
        new_daemon -> args = malloc(sizeof(char *));
        if (new_daemon -> args == NULL) {
            fprintf(out, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        new_daemon->args[0] = strdup(args[0]);
        if (new_daemon->args[0] == NULL) {
                fprintf(out, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
    }
     else {
        new_daemon->args = NULL;
    }

    // Reallocate memory for the daemons array to include the new daemon
    Daemon *temp = realloc(daemons_g, (num_daemons_g + 1) * sizeof(Daemon));
    if (temp == NULL) {
        fprintf(out, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    daemons_g = temp;

    // Add the new daemon to the array
    daemons_g[num_daemons_g++] = *new_daemon; // Note: Here we're copying the struct, not the pointer
    //fprintf(out, "Normal return!\n");
    return new_daemon;
}


// Method to delete daemon --> free memory and delete pointer to it from array daemons
int delete_daemon(char *name) {
    // Find the daemon with the given name
    Daemon *daemon_to_delete = find_daemon(name);
    if (daemon_to_delete == NULL) {
        // Daemon not found
        return -1; // error
    }

    // Free memory allocated for the daemon's name
    free(daemon_to_delete->name);
    daemon_to_delete->name = NULL;

    // Free memory allocated for the daemon's status
    free(daemon_to_delete->status);
    daemon_to_delete->status = NULL;

    // Free memory allocated for the daemon's arguments
    if ((daemon_to_delete -> args != NULL) && (daemon_to_delete -> num_args != 0) ){
        for (int i = 0; i < daemon_to_delete -> num_args; i++) {
            free(daemon_to_delete->args[i]);
        }
        free(daemon_to_delete->args);
        daemon_to_delete->args = NULL;
    }

    // Shift remaining daemons to fill the gap
    for (size_t i = 0; i < num_daemons_g; i++) {
        if (&daemons_g[i] == daemon_to_delete) {
            for (size_t j = i; j < num_daemons_g - 1; j++) {
                daemons_g[j] = daemons_g[j + 1];
            }
            num_daemons_g--;

            // Reallocate memory for the daemons array
            Daemon *temp = realloc(daemons_g, num_daemons_g * sizeof(Daemon));
            if (temp == NULL && num_daemons_g > 0) {
                fprintf(stderr, "Memory reallocation failed\n");
                exit(EXIT_FAILURE);
            }
            daemons_g = temp;

            return 0; // daemon deleted successfully
        }
    }

    // Error
    return -1; // TODO: Handle error properly
}

// Method to print the content of the arguemtns array of strigns pf the users_input data structure
void print_arguments(char **args, int size) {
    if (args == NULL) {
        printf("Arguments: NULL\n");
        return;
    }

    printf("Arguments:\n");
    for (int i = 0; i < size; i++) {
    
        //if (args[i] == NULL){
        //    printf("  %s\n", "-NULL-" );
        //}
        
        //else{
        printf("  %d: %s\n", i, args[i]);
        //}
    }
    //printf("  %s\n", "-end-" );
}

void print_daemons() {
    printf("Daemon List:\n");
    printf("-------------------------------------------------\n");
    for (int i = 0; i < num_daemons_g; i++) {
        printf("Daemon %d:\n", i + 1);
        printf("Name: %s\n", daemons_g[i].name);
        printf("PID: %d\n", daemons_g[i].pid);
        printf("Status: %s\n", daemons_g[i].status);
        printf("Num of args: %d\n", daemons_g[i].num_args);
        printf("Arguments:\n");
        
        if (daemons_g[i].args != NULL) {
            for (int j = 0; j < daemons_g[i].num_args; j++) {
                printf("  %s\n", daemons_g[i].args[j]);
            }
        } else {
            printf("  None\n");
        }
        printf("-------------------------------------------------\n");
    }
}

// Method to terminate all the daemons before quiiting the program: 
void terminate_all_daemons(FILE *out, FILE *in){
    // Iterate through all daemons in the daemons_g list: 
    for (int i = 0; i < num_daemons_g; i++) {
        // get a daemon: 
        Daemon *daemon = &daemons_g[i];
        if (strcmp(daemon -> status, "inactive") != 0){
            stop_c(out, in, daemon -> name);
        }
        // stop daemon:      
    }
}
/*
// Get a string for a status to print: 
char *get_status_str(enum daemon_status status) {
   switch (status) 
   {
      case status_unknown: return "status_unknown";
      case status_inactive: return "status_inactive";
      case status_starting: return "status_starting";
      case status_active: return "status_active";
      case status_stopping: return "status_stopping";
      case status_exited: return "status_exited";
      case status_crashed: return "status_crashed";
   }
   // shoudb't reach it here:
   return NULL;
}
*/

