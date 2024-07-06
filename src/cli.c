/*
 * Legion: Command-line interface
 */

#ifndef LEGION_H // TODO: !!!!!!!!!!!!!!!!!!!!!!! if I dont add it it throws errors, what can I do to avoid it? 
#define LEGION_H
#include "legion.h"
#endif
#include "main_helper.h" // TODO: remove this, anyways will be replaced, this file (it won't actually, yes? )
#include "processes.h"
#include <sys/stat.h> // for mkdir

#define PROMPT "legion> "

void run_cli(FILE *in, FILE *out){
    char *input_line = ""; 
    users_input *parsed_input; 
    char *command; 
    
    // Cate the logfile directory: (if exists, don't )
    if (access(LOGFILE_DIR, F_OK) == -1) {
        // Directory doesn't exist, create it
        if (mkdir(LOGFILE_DIR, 0777) == -1) {
            // Handle error if mkdir fails
            fprintf(out, "%s","Failed to create log directory");
            sf_error("Failed to create log directory");
            exit(EXIT_FAILURE);
        }
    }
    //input_line = init_users_input(in, out);
    
    
    // STEP 1: open the file in // TODO: should I open the file? 
    
    // STEP 1: While loop? for printing the "legion> " prompt if not quit
    
        // If quit ---> kill every child and quit 
    
    // STEP 2: Parsing the command line once we get it               --> parse method
    
    // STEP 3: Switch for every command 
     
   //signal(SIGQUIT, SIG_IGN); // to ignore interruptions TODO: ? ??
   
   // Signal Handlers:
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        perror("Failed to register SIGCHLD handler");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Failed to register SIGINT handler");
        exit(EXIT_FAILURE);
    } 
    
    sigset_t prev_mask;
    sigprocmask(SIG_BLOCK, NULL, &prev_mask);

    
    while(!sigint_received) {
      
        //printf("\nnum daemons: %d\n", num_daemons_g);
        //print_daemons();
        sf_prompt(); // TODO: IS IT HERE?
        fprintf(out, PROMPT);
        fflush(out);
        input_line = get_input_line(in, out);
        //printf("\ninput line: %s\n", input_line); // TODO: remove this!!!!!!!!!
	    if (strlen(input_line) == 0) {
            free(input_line);
            continue; // start the loop again
        }
        
        // Get parsed input stored in the user_input ddata structure: 
        //printf("\nREACHED HERE x10: array size: %s!\n", input_line); // TODO: Remove it !!!!!!!!!  
        parsed_input = parse_input(out, input_line);
        //printf("\n%s\n", "REACHED PAST INPUT PARSING!"); // TODO: Remove it !!!!!!!!!
        free(input_line); // can free it now, don't need it anymore
        if (parsed_input == NULL){ // error!!!  
            //printf("\n%s\n", "ERROR: invalid input!"); // TODO: replace it with relevant error handling!
            free_users_input(parsed_input);
            //return; // TODO: WHAT I AM DOING ON ERROR HERE? 
            continue;
        }
        command = parsed_input -> command;      
        if(strcmp(command, "help") == 0){                      
            help_c(out);
            //return;
        }     
        else if (strcmp(command, "status-all") == 0){
            if (status_all_c(out) == -1){
                sf_error("Error executing command");
                fprintf(out, "Error executing command: %s\n", command);   
            }
        }
        else if (strcmp(command, "status") == 0){             
            //sf_status(char *msg); // TODO: what is this msg? !!!!!!!!!   
            if (status_c(parsed_input -> name, out) == -1){
                //printf("\n%s\n", "ERROR: status!"); // TODO: replace it with relevant error handling
                sf_error("Error executing command");
                fprintf(out,"\nError executing command: %s\n", command);
                // TODO: handle error
            }
            //continue;
            //return;  
        }
        else if (strcmp(command, "quit") == 0){  
            terminate_all_daemons(out, in);       
            //sf_fini(); // TODO: Do we have to include this here? 
            // TODO TODO TODO TODO TODO : terminate all the child processes before quiting !!!!!!!!!!!!!!!!!!!!
            break; 
        }
        else if (strcmp(command, "start") == 0){
            if (start_c(out, in, parsed_input -> name) == -1){
                sf_error("Error executing command");
                fprintf(out, "Error executing command: %s\n", command);                   
            }
            //else {//printf("Normal return!!!!!!!\n");} // TODO: remove this!!!! and handle errors normally!!!!!!!
        }
        else if (strcmp(command, "stop") == 0){
            if (stop_c(out, in, parsed_input -> name) == -1){
                sf_error("Error executing command");
                fprintf(out, "Error executing command: %s\n", command);    
            }
        }
        else if (strcmp(command, "unregister") == 0){
            if (unregister_c(out, parsed_input -> name) == -1){
                sf_error("Error executing command");
                fprintf(out, "Error executing command: %s\n", command);   
            }
            else { sf_unregister(parsed_input -> name);}
        }
        else if (strcmp(command, "register") == 0){           
            if(register_c(out, parsed_input -> name, parsed_input -> arguments[1], parsed_input -> arguments, parsed_input -> num_args) == NULL){
                //printf("\nREACHED HERE x3: array size: %d !\n", array_size(parsed_input -> arguments)); // TODO: Remove it !!!!!!!!!  
                sf_error("Error executing command");
                fprintf(out, "Error executing command: %s\n", "register");
                //free_users_input(parsed_input);
                //free(command);
                // TODO: ERROR!!!!!!!
                //return;
                //break;
                
            }
            else{sf_register(parsed_input -> name, parsed_input -> arguments[0]);} //TODO: need to incude this!!!!!!!!!!
            //printf("\nREACHED HERE x7: array size: %d !\n", array_size(parsed_input -> arguments)); // TODO: Remove it !!!!!!!!!  
        }
        
        else if (strcmp(command, "logrotate") == 0){
            logrotate_c(out, parsed_input -> name);
        }
        
        
    free(command);   
    //printf("\nREACHED HERE x8: array size: %s !\n", "hm"); // TODO: Remove it !!!!!!!!!  
    free_users_input(parsed_input);  
    //free(input_line); 
     
    }
   if(sigint_received == 1){
        // handle sigint: 
        terminate_all_daemons(out, in);
        
        }
}

// valgrind --leak-check=full --trace-children=yes bin/legion_tests -j1
