#pragma once

#ifndef MAIN_HELPER_H
#define MAIN_HELPER_H

#include "helper.h"
#include "command.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct users_input users_input;

char *get_input_line(FILE *in, FILE *out);
char **tokenize_input(char *input_to_tokenize, int *size_of_tokens);
void free_list_of_tokens(char **list_of_tokens, int num_tokens);
users_input *parse_input(FILE *out, char *input_to_parse);

int array_size(char **arr);
int is_command(char *str);
char **copy_strings_from_index(const char **source, int source_size);


#endif
