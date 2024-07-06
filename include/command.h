#pragma once

#ifndef COMMAND_H
#define COMMAND_H

#include <signal.h>

#ifndef LEGION_H // TODO: !!!!!!!!!!!!!!!!!!!!!!! if I dont add it it throws errors, what can I do to avoid it? 
#define LEGION_H
#include "legion.h"
#endif
#include "main_helper.h"
#include "helper.h"


typedef struct daemon Daemon;

// Need some structure to store the users input command line to be parsed: 
// TODO: Do not forget to free this after you are calling an appropriate command and parsing the arguements
typedef struct users_input{
    //char *raw_input; // the raw input intself
    char *command; 
    char *name; // popular arguemtn that is either present or not. if not present --> NULL
    // --> additional arguemtns for register command that include executable name and any additional arguments to be parsed later by the register function itself!
    char **arguments; 
    int num_args;
    //int valid; // is set after validation step 
} users_input;

users_input *init_users_input();   
void free_users_input(users_input *input);

// Commands methods: 
int status_c(char *name, FILE *out);
void help_c(FILE *out);
void quit_c(FILE *out);
Daemon *register_c(FILE *out, char *name, char *exe_name, char **args, int num_args);
int status_all_c(FILE * out);

//int start_c(FILE *out, FILE *in, char * name);

int unregister_c(FILE *out, char *name);

void logrotate_c(FILE *out, char *daemon_name);


#endif
