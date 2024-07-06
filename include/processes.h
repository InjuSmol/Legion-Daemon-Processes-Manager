#pragma once

#ifndef PROCESSES_H
#define PROCESSES_H

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "helper.h"
#include <sys/wait.h> // for wait
#include <fcntl.h> // for open 
#include <limits.h> // for PATH_MAX
#include <errno.h>
 
extern volatile sig_atomic_t sigint_received;


void handle_alarm(int sig);

void sigchld_handler(int signum);
void sigint_handler(int signum);

int start_c(FILE *out, FILE *in, char * name);
char **modify_environment();

int stop_c(FILE *in, FILE *out, char* name);

#endif
