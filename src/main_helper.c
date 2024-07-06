//#include "legion.h"
#include "main_helper.h"
//#include "command.c"


char *commands_list[] = {"start", "quit", "register", "unregister", "help", "status", "status-all", "stop", "logrotate"};

// returns the users input line as a single string to be parsed in the next step 
char *get_input_line(FILE *in, FILE *out) {
    char *input_line = NULL;
    size_t input_size = 0;
    ssize_t characters_read;
    
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Failed to register SIGINT handler");
        exit(EXIT_FAILURE);
    }


    // Unblock SIGINT
    struct sigaction sa;
    sigset_t smask;
    sigemptyset(&smask);
    sa.sa_handler = sigint_handler;
    sa.sa_mask = smask;
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(out,"%s\n","Failed to register SIGINT handler");
        exit(EXIT_FAILURE);
    }
    
    sigset_t prev_mask;  // Variable to store the previous signal mask:
    sigprocmask(SIG_BLOCK, &smask, &prev_mask);  // Block signals and save previous mask

    characters_read = getline(&input_line, &input_size, in);
    if (characters_read == -1) {
        // Handle the error
        //fprinf(out,"%s\n","Failed to register SIGINT handler");
        sf_error("Error reading command");
        sf_fini();
        terminate_all_daemons(out, in);exit(0);
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);  // Restore previous signal mask
        //return input_line;
    }

    if (sigint_received == 1){terminate_all_daemons(out, in);exit(0);}
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);  // Restore previous signal mask
    //sigprocmask(SIG_BLOCK, &sigset, NULL);
    // Remove newline character at the end of the input line, if present:
    if (input_line[characters_read - 1] == '\n') {
        input_line[characters_read - 1] = '\0';
    }
    return input_line;
}

// MEthod to check if a string is one of the 

// Method to tokenize the input_line_to_parse
// Returns an array of strings separated either with spaces, or single quotes with spaces or the end of input: 
char **tokenize_input(char *input_to_tokenize, int *size_of_tokens) {
    char **list_of_tokens = NULL;
    char *input_ptr = input_to_tokenize;
    char *ptr_to_token_start = NULL;
    char *ptr_to_token_end = NULL;
    int first_s_q_passed = 0; // indicates if we have already passed a "valid" opening single quote
    int num_tokens = 0; // counter to keep track of the number of tokens

    // Iterate through every char in the input line:
    while (*input_ptr != '\0') {
        // Skip leading spaces:
        while (isspace(*input_ptr) && (first_s_q_passed == 0)) {
            input_ptr++;
        }

        // Initialize token start and end pointers:
        ptr_to_token_start = input_ptr;
        ptr_to_token_end = input_ptr;

        // Read the token:
        while (*ptr_to_token_end != '\0') {
            if (*ptr_to_token_end == '\'') {
                if (first_s_q_passed == 0 && *(ptr_to_token_end-1) == ' ' && (!(*(ptr_to_token_end+1) == ' '))) {
                    first_s_q_passed = 1; // mark the start of a 'long argument'
                } else if (first_s_q_passed == 1 && *(ptr_to_token_end+1) == ' ') {
                    first_s_q_passed = 0; // mark the end of a 'long argument'
                    ptr_to_token_end++;
                    break;
                }
            }
            if (isspace(*ptr_to_token_end) && first_s_q_passed == 0) {
                break; // end of token
            }
        
            ptr_to_token_end++;
        }

        // Allocate memory for the token:
        int token_length = ptr_to_token_end - ptr_to_token_start;
        char *token = (char *)malloc((token_length + 1) * sizeof(char));
        if (token == NULL) {
            perror("Error allocating memory for token");
            exit(EXIT_FAILURE);
        }

        // Copy the token character by character:
        char *token_ptr = token;
        for (char *ch = ptr_to_token_start; ch < ptr_to_token_end; ch++) {
            if (*ch != '\'') {
                *token_ptr++ = *ch;
            }
        }
        *token_ptr = '\0'; // null-terminate the token

        // Add the token to the list of tokens:
        list_of_tokens = (char **)realloc(list_of_tokens, (num_tokens + 1) * sizeof(char *));
        if (list_of_tokens == NULL) {
            perror("Error reallocating memory for token list");
            exit(EXIT_FAILURE);
        }
        list_of_tokens[num_tokens] = token;
        num_tokens++;

        // Move to the next character after the current token:
        input_ptr = ptr_to_token_end;
    }

    // Update the size of the tokens list
    *size_of_tokens = num_tokens;

    // Print the tokens list (optional)
    //print_tokens(list_of_tokens, num_tokens);

    return list_of_tokens;
}

void free_list_of_tokens(char **list_of_tokens, int num_tokens) {
    // Iterate through the list of tokens:
    for (int i = 0; i < num_tokens; i++) {
        // Free the memory allocated for each token:
        free(list_of_tokens[i]);
    }
    // Free the memory allocated for the list of tokens itself:
    free(list_of_tokens);
}

// Method to parse the input users line into command, name (if present as first argument and other arguemnts as a single string):
// On error the method returns NULL (it is assumed that only non-zero input is passed to this method and it is checked prior to calling this funciton )
users_input *parse_input(FILE *out, char *input_to_parse){
    int num_arguments = 0; // if the num_tokens is less than 3 => 0, if >= 3, then num_tokens -2
    int num_tokens = 0;
    users_input *parsed_input; // if error will return NULL
    
    // STEP 1: Tokenize the input line
    char **list_of_tokens = tokenize_input(input_to_parse, &num_tokens);
    if (num_tokens >= 3){
        num_arguments = num_tokens -2;
    }
    
    // Number of tokens we got: 
    //int tokens_num = array_size(list_of_tokens);
    
    // STEP 2: initialize the user_input object to store our parsed situation 
    parsed_input = init_users_input();
    
    /*
    // Free list of tokens: 
    if (list_of_tokens != NULL) {// just in case check
        free_list_of_tokens(list_of_tokens);
        free_users_input(parsed_input);
        //return NULL;
    }
    */
    // STEP 3: Check if it is valid: 
        
    // STEP 4: Check if the first argument is one of the commands from commands list  
    if(!(is_command(*list_of_tokens))){ // first token 
        free_list_of_tokens(list_of_tokens, num_tokens);
        free_users_input(parsed_input);
        return NULL; // invalid input 
    }
    parsed_input -> num_args = num_arguments;
    
    parsed_input -> command = list_of_tokens[0];
     //printf("\n%s\n", "REACHED HERE x20!"); // TODO: Remove it !!!!!!!!!
    //printf("\nthe command: %s", parsed_input -> command); // TODO: don't forget to remove this!!!!!!!
     //printf("\n%s\n", "REACHED HERE x20!"); // TODO: Remove it !!!!!!!!!
    // STEP 5: If the first arguement is help or quit, then just store it in command and return: Doesn't require an argument
    if (((strcmp(parsed_input -> command, "help") == 0) || (strcmp(parsed_input -> command, "quit") == 0)) || (strcmp(parsed_input -> command, "quit") == 0) || (strcmp(parsed_input -> command, "status-all") == 0)){
        return parsed_input;
    }
    // If the first arguemtn is a command that requires 1 argument: 
    else if ((strcmp(parsed_input -> command, "unregister") == 0) ||
       (strcmp(parsed_input -> command, "status") == 0) ||
       (strcmp(parsed_input -> command, "start") == 0) ||
       (strcmp(parsed_input -> command, "stop") == 0) ||
       (strcmp(parsed_input -> command, "logrotate") == 0)){
           // check if there is only two elements in the list_of_tokens + store the second token under the name: 
           if (array_size(list_of_tokens) != 2){              
               fprintf(out, "Wrong number of args (given: %d, required: 1) for command '%s'",(array_size(list_of_tokens) - 1), parsed_input -> command); // TODO: printf?
               sf_error("Error executing command");
               fprintf(out, "\nError executing command: %s\n", parsed_input -> command);
               free_list_of_tokens(list_of_tokens, num_tokens);
               free_users_input(parsed_input);
               return NULL;
           }
           parsed_input -> name = list_of_tokens[1];    
           return parsed_input;      
    } 
    // if first arguement is a command that requires two or more arguements:  
    else if (strcmp(parsed_input -> command, "register") == 0){
        // if # of arguments is 0: (need at least 1 --> exe name )
        if (num_arguments == 0){
               free_list_of_tokens(list_of_tokens, num_tokens);
               free_users_input(parsed_input);
               //fprintf(out, "Wrong number of args (given: %d, required: 2) for command '%s'",(array_size(list_of_tokens) - 1), parsed_input -> command); // TODO: printf?       
               sf_error("Error executing command");
               printf("\nError executing command: %s\n", parsed_input -> command);
               return NULL; // at least must be 2 arguements + command itself => command + name of daemon + executable      
        } 
        /*       
        // Check if a daemon with such a name is already registered: 
        else if (find_daemon(parsed_input -> name) != NULL ){
            fprintf(out, "Daemon %s is already registered.", parsed_input -> name);
            return NULL; 
             
        }
        */
        // If the number of arguments provided is 3
        /*
        else if (num_arguments == 1){
            //printf("\n%s\n", "REACHED HERE x4!"); // TODO: Remove it !!!!!!!!!
            // Add the exe name into list of arguments in the first position: 
            parsed_input->arguments = malloc(sizeof(char *)); // allocate memory for just one string pointer
            if (parsed_input->arguments == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }  
            // put the executabe name to the first position of the argument vector:   
            parsed_input -> arguments[0] = strdup(list_of_tokens[2]);
        }  
        */  
        // If not, store its name and argv argument vector in the parsed_input:       
        else {
        //printf("\n%s\n", "REACHED HERE x21!"); // TODO: Remove it !!!!!!!!!
            //print_arguments(parsed_input -> arguments,(array_size(list_of_tokens))); // TODO: remove this
            parsed_input -> name = list_of_tokens[1];
            //int number_of_arguments = 0; 
            //number_of_arguments = (array_size(list_of_tokens) - 2);
            //printf("\n #: %d\n", number_of_arguments); //TODO: REMOVE THIS!!!!
            // store the argv in arguments: but need to parse them first: argv all all the [num_of_tokens - 2 --> staring from 3rd elemnts in the tokens_list]
                parsed_input->arguments = malloc((num_arguments)* sizeof(char *));
                if (parsed_input->arguments == NULL) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }   
                //printf("\n list of tokens: %d vs arguements: %d\n", array_size(list_of_tokens)-2, array_size(parsed_input -> arguments)); // TODO: remove this
                for (int i = 0; i < num_arguments; i++) {
                    parsed_input -> arguments[i] = strdup(list_of_tokens[i + 2]);
                    if (parsed_input->arguments[i] == NULL) {
                        perror("Memory allocation error");
                        free_list_of_tokens(list_of_tokens, num_tokens);
                        free_users_input(parsed_input);
                        return NULL;
                    }
                    //printf("\n- %d - %s\n", (i+2), parsed_input -> arguments[i] ); // TODO: remove this 
                }
        }
        //printf("\n list of tokens: %d vs arguements: %d\n", array_size(list_of_tokens)-2, array_size(parsed_input -> arguments)); // TODO: remove this
        //print_arguments(parsed_input -> arguments, num_arguments); // TODO: remove this
        //printf("\n%s\n", "reached here x5"); // TODO: remove this
        return parsed_input; 
    }  
    // STEP 6: If the first arguemnt is start, store the start in command and the name in name and etc.     
    // STEP 7: On errors: free the user_input and return NULL --> invalid input    
    free_list_of_tokens(list_of_tokens, num_tokens);
    free_users_input(parsed_input);
    return NULL;
}

 /*
typedef struct input{
    //char *raw_input; // the raw input intself
    char *command; 
    char *name; // popular arguemtn that is either present or not. if not present --> NULL
    // --> additional arguemtns for register command that include executable name and any additional arguments to be parsed later by the register function itself!
    char *arguments; 
    //int valid; // is set after validation step 
} users_input;
*/

// method to determine if a string is one of the valid commands: 
int is_command(char *str) {
    for (int i = 0; i < array_size(commands_list); i++) {
        if (strcmp(str, commands_list[i]) == 0) {
            return 1; // command found
        }
    }
    return 0; // command not found
}

// Method to determine the size of an array of strings: 
int array_size(char **arr) {
    size_t size = 0;
    while (arr[size] != NULL) {
        size++;
    }
    return size;
}

// Method to copy strings starting from the 3rd index into another array:
char **copy_strings_from_index(const char **source, int source_size) {
    // Calculate the size of the new array
    int new_size = source_size - 2; // Exclude the first two strings

    // Allocate memory for the new array of strings
    char **new_array = (char **)malloc(new_size * sizeof(char *));
    if (new_array == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    // Copy strings starting from the 3rd index into the new array
    for (int i = 2; i < source_size; i++) {
        new_array[i - 2] = strdup(source[i]);
        if (new_array[i - 2] == NULL) {
            perror("Memory allocation error");
            exit(EXIT_FAILURE);
        }
    }

    return new_array;
}


// ex. " help otherarguemnts to be ignored"

// ex. " quit "

// ex. " register lazy path/to/name.exe 'argumentvector_argv argumetn'" --> single quotes: if " '" => until meet "' " or " '\0" (the ' in the end of the string ) or end of the input, put it in one string

// ex. "unregister polly "

// ex. "start "


