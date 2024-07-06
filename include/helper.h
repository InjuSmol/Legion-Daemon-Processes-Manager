#ifndef HELPER_H
#define HELPER_H

#ifndef LEGION_H // TODO: !!!!!!!!!!!!!!!!!!!!!!! if I dont add it it throws errors, what can I do to avoid it? 
#define LEGION_H
#include "legion.h"
#endif
#include "main_helper.h"
#include "command.h"
#include <string.h>
#include "processes.h"


typedef struct daemon Daemon;

extern int num_daemons_g;

extern Daemon *daemons_g;

//typedef enum daemon_status daemon_status;

typedef struct daemon{
   char *name; 
   pid_t pid; // TODO: is it right? 
   char *status;
   char** args; // Arguments for execution: include the executable name in the first place!!!!!!!!!!!
   int num_args;
      // TODO: is it right?    
} Daemon;

void print_tokens(char **list_of_tokens, int num_tokens);

// Getter methods: 
Daemon *find_daemon(char *name);
char *get_status(char *name);
void set_status(FILE *out, Daemon *daemon, char *new_status);
pid_t get_daemon_pid();
Daemon *create_daemon(FILE *out, char *name, char *status, pid_t pid, char **args, int num_args);
int delete_daemon(char *name);
//char* get_status_str(daemon_status status);
void print_arguments(char **args, int size);
void print_daemons();
void terminate_all_daemons(FILE *out, FILE *in);

/**
 * `unknown`, if the specified name is not the name of a currently registered daemon,
 * `inactive`, if the name has been registered, but no daemon process has been started,
 * `starting`, if the daemon is being started but has not yet synchronized with
   its parent to indicate that it is fully up (see below).
 * `active`, if the daemon process has been started and is currently running.
 * `stopping`, if we have sent the daemon a `SIGTERM` signal to request its termination,
    but we have not yet been notified that termination has occurred.
 * `exited`, if the daemon process has terminated by calling `exit()`.
 * `crashed`, if the daemon process has terminated abruptly as a result of a signal.
**/


#endif
