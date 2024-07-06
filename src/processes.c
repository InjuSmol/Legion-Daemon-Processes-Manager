#include "processes.h"


// a flag to set when the sync message is recieved:
volatile sig_atomic_t sync_msg_received = 0;

// a flag to inidacte if SIGCHLD signal is received:
volatile sig_atomic_t sigchld_received = 0;

volatile sig_atomic_t sigint_received = 0;

//Current environment
extern char **environ;

// Method to handle SIGCHLD signal:
void sigchld_handler(int signum) {
    sigchld_received = 1;
}


// Method to handle SIGINT signal:
void sigint_handler(int signum) {

    sigint_received = 1;
}


// Method to handle recieving of an alarm signal: 
void handle_alarm(int sig) {
    sync_msg_received = 0; // Reset flag
} 

// Method to modify the environment:
char **modify_environment() {
    // Copy the current environment:
    char **envp = environ;

    // Find the index of the PATH environment variable:
    int path_index = -1;
    for (int i = 0; envp[i] != NULL; i++) {
        if (strncmp(envp[i], PATH_ENV_VAR "=", strlen(PATH_ENV_VAR) + 1) == 0) {
            path_index = i;
            break;
        }
    }
    // If PATH environment variable is found, modify it:
    if (path_index != -1) {
        // Get the current value of PATH:
        char *path_value = envp[path_index] + strlen(PATH_ENV_VAR) + 1; // skip "PATH=" prefix

        // Allocate memory for the modified PATH value:
        size_t new_path_len = strlen(DAEMONS_DIR) + strlen(path_value) + 2; // +2 for colon and null terminator
        char *new_path = malloc(new_path_len);
        if (new_path == NULL) {
            perror("Memory allocation error");
            exit(EXIT_FAILURE);
        }
        // Construct the new PATH value with DAEMONS_DIR prepended:
        snprintf(new_path, new_path_len, "%s:%s", DAEMONS_DIR, path_value);

        // Update the environment with the modified PATH value:
        envp[path_index] = new_path;
    }
    // Return the modified environment:
    return envp;
}

// Method to start a daemon process: 
int start_c(FILE *out, FILE *in, char * name){

// STEP 1: deamon: ----------------------------------------------------------------------------------------

    sync_msg_received = 0;

    // get a pointer to the daemon you need to start: 
    Daemon *daemon = find_daemon(name);
    
    if (daemon == NULL){
        fprintf(out, "Daemon %s is not registered.\n", name);
        return -1; // error !!!!!!!! // TODO: handle the errror!!!!
    }
    
    // validate the status of the daemon to be 'inactive':
    if (strcmp(daemon -> status, "inactive") != 0){
        return -1; // error !!!!!!!! // TODO: handle the errror!!!!
    }
       
    // Set a new 'starting' status: 
    set_status(out, daemon, "starting");
    sf_start(name);

// STEP : initialize a mask set:  --------------------------------------------------------------------------   
    // Initialize and fill a signal set mask_all with all possible signals:
    sigset_t mask_all, prev_all; // prev_all -> to store the previous signal mask to restore later
    if (sigfillset(&mask_all) == -1) {
        set_status(out, daemon, "inactive"); // TODO: NOT SURE IF I NEED TO DO THAT BEFPRE EACH EXIT_FAILURE
        perror("sigfillset error");
        exit(EXIT_FAILURE);
    }
    
// STEP 2: pipe & fork:  -----------------------------------------------------------------------------------
    
    // the name for the 
    char log_fd_name[PATH_MAX];
    snprintf(log_fd_name, sizeof(log_fd_name), "%s/%s.log.0", LOGFILE_DIR, daemon -> name);

    // Open or create the log file:
    int log_fd = open(log_fd_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        set_status(out, daemon, "inactive");
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
   
    int fd[2]; // File descriptors for the pipe ! -> fd[0] - read, fd[1] -> write
    pid_t pid;
     
    // Create a pipe for synchronization: 
    if (pipe(fd) == -1) {
        set_status(out, daemon, "inactive");
        fprintf(out, "%s","Failed to create a pipe");
        exit(EXIT_FAILURE);
    }
    
    // TODO:Do i need to put it exactly here? Don't really understand the purpose of it: 
    
    
    // Block signals before forking:
    if (sigprocmask(SIG_BLOCK, &mask_all, &prev_all) == -1) {
        set_status(out, daemon, "inactive");
        fprintf(out, "%s","sigprocmask error");
        exit(EXIT_FAILURE);
    }
    

    // Fork a child process:
    pid = fork();
    if (pid == -1) {
        // Forking failed
        set_status(out, daemon, "inactive");
        fprintf(out, "%s","Fork failed");
         if (sigprocmask(SIG_SETMASK, &prev_all, NULL) == -1) {
            fprintf(out, "%s","sigprocmask error");
            exit(EXIT_FAILURE);
        }
        // Reset the status of the daemon to inactive
        return -1;
 
// STEP : CHILD PROCESS   ----------------------------------------------------------------------------------       
    } else if (pid == 0) { // child process 
        // Unblock signals: // TODO: need it here? 
        
        if (sigprocmask(SIG_SETMASK, &prev_all, NULL) == -1) {
            set_status(out, daemon, "inactive");
            fprintf(out, "%s","sigprocmask error");
            exit(EXIT_FAILURE);
        }
        
        //By calling setpgid in the child process, you're ensuring that it's not affected by signals (such as SIGINT, SIGQUIT, or SIGTSTP) generated from the terminal.
        if (setpgid(getpid(), getpid())<0) {
            set_status(out, daemon, "inactive");
            fprintf(out, "%s","setpgid error");
            exit(EXIT_FAILURE);
        }
         
        // Redirect stdout to the log file:
        if (dup2(log_fd, STDOUT_FILENO) == -1) { // standard output --> 1
            set_status(out, daemon, "inactive");
            fprintf(out, "%s","Failed to redirect stdout");
            exit(EXIT_FAILURE);
        } 
        
        close(fd[0]); // close the read side of the pipe in the child process

        // If execvpe returns, it failed, handle the error
        /*
        // write to the fd[1]
        char sync_msg = '?'; // What kinda message it must be? 
        if (write(fd[1], &sync_msg, sizeof(char)) == -1) {
            perror("Write to pipe failed");
            exit(EXIT_FAILURE);
        }
        */  // ====> replaced it with redirection to the executable process to handle the sync message: 
              
        // Redirect stdout to the write end of the pipe ---> executable of the child process is going to take care of the sync msg sending: 
        if (dup2(fd[1], SYNC_FD) == -1) {
            perror("Failed to redirect stdout to SYNC_FD");
            exit(EXIT_FAILURE);
        }
               
        /* TODO: Where must it be? 
        close(log_fd); // close the original file descriptor for the log file
        close(fd[1]); // close the write side of the pipe in the child process
        */     
        // Restore the full path to the executable: 
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", DAEMONS_DIR, daemon->args[0]);
        // TODO: hre i need to pass a copy, shouldn't modify the args directly '
        // new full_path to the exe file:
        daemon->args[0] = strdup(full_path);
       
        // New environment to pass to execvpe:
        char **envp = modify_environment();
         /*
        if (access(full_path, X_OK) == -1) {
            // Handle error: file doesn't exist or is not executable
            perror("Error accessing file");
            exit(EXIT_FAILURE);
        } 
        */
        //fprintf(out,"full path: %s\n", full_path);  // TODO: remove this
               
        // Add a null pointer at the end of the array
        daemon->args[daemon->num_args] = NULL;
                
        // Pass the modified args array to execvpe function call
        execvpe(full_path, daemon->args, envp);
               
        close(fd[0]); // close the write end of the synchronization pipe
           
        close(fd[1]); // close the write end of the synchronization pipe
    
       
        close(log_fd);
        // execvpe returns only on error
        if (kill(pid, SIGKILL) == -1) {
            perror("Failed to kill child process");
            exit(EXIT_FAILURE);
        }
        sf_kill(name, pid);
        sf_error("Error: execvpe failed");
        exit(EXIT_FAILURE);
        
        
// STEP : PARENT PROCESS  ----------------------------------------------------------------------------------
    } else {
        
        // Unblock signals after forking in parent process: 
        if (sigprocmask(SIG_SETMASK, &prev_all, NULL) == -1) {
            fprintf(out, "%s", "Error: sigprocmask error");
            exit(EXIT_FAILURE);
        }
       
            
        close(fd[1]); // close the write end of the synchronization pipe
    
        close(log_fd);
        
        signal(SIGALRM, handle_alarm); // TODO: implement this 
        alarm(CHILD_TIMEOUT);
        
        //printf("%d\n", sync_msg_received);
        
        // Read from the pipe: 
        char sync_msg;  // TODO: is it okay they are declared twice? 
// STEP : recieve the sync msg:  ----------------------------------------------------------------------------------        
        while (!sync_msg_received) {
            if (read(fd[0], &sync_msg, sizeof(char)) <= 0) {
                 // Set status of the daemon to inactive:
                set_status(out, daemon, "inactive"); // TODO: need it here? 
                fprintf(out, "%s","Error: Read from pipe failed");
                //exit(EXIT_FAILURE);
                break;
            }
            else{
            sync_msg_received = 1;
            alarm(0); // cancels alarm after the message is recieved
            }
        }      
        
// STEP : sync_msg not recieved:  ----------------------------------------------------------------------------------          
        if (!sync_msg_received) {
            // Terminate the child process using SIGKILL:
            if (kill(pid, SIGKILL) == -1) {
                fprintf(out, "%s","Error: Failed to kill child process");
                exit(EXIT_FAILURE);
            }
            sf_kill(name, pid);
                  
            // block signals: 
            if (sigprocmask(SIG_BLOCK, &mask_all, &prev_all) < 0) { // block
                fprintf(out, "%s","Error: sigprocmask");
                exit(EXIT_FAILURE);
            }     

            // Set status of the daemon to inactive:
            set_status(out, daemon, "inactive");
            
            // TODO: handle the timeout error (e.g., print an error message)
            fprintf(stderr, "Synchronization not received within the timeout period. Terminating child process.\n");
            if (sigprocmask(SIG_SETMASK, &prev_all, NULL) < 0) { // unblock
                fprintf(out, "%s","Error: Sigprocmask");
                exit(EXIT_FAILURE);
            }     
            close(fd[0]); // Close the read end of the pipe in the parent process 
            //printf("%s\n", "here here here"); // Remove this !!!!!!
            return -1; // errror TODO: what to do on error? 
            
        }
        
// STEP : sync_msg is recieved:  ---------------------------------------------------------------------------------- 
        // if sync_msg is recieved: 
        else{    
            // block signals: 
            if (sigprocmask(SIG_BLOCK, &mask_all, &prev_all) < 0) { // block
                fprintf(out, "%s","Error: sigprocmask");
                exit(EXIT_FAILURE);
            }     
            // Set status to 'active'
            set_status(out, daemon, "active");
            sf_active(name, pid);
            daemon -> pid = pid; 
            //fprintf(out, "Received sync msg: %c\n", sync_msg); // TODO: remove this!!!!!
            
            // Unblock signals: 
            if (sigprocmask(SIG_SETMASK, &prev_all, NULL) < 0) { // unblock
                fprintf(out, "%s","Error: sigprocmask");
                exit(EXIT_FAILURE);
            } 
                                                          
        // TODO TODO TODO TODO: Do I need to do that in start()? here? 
// STEP : reap child process:  ---------------------------------------------------------------------------------- 
            // Wait for the child process to terminate: 
            
            if (waitpid(pid, NULL, WNOHANG) == -1) {   //TODO: not sure about that? ----> make sure on how the waitpid works 
                fprintf(out, "%s","Failed to wait for child");
                exit(EXIT_FAILURE);
            }
            
             close(fd[0]); // Close the read end of the pipe in the parent process
             return 0;    
        }        
// STEP : parent process ends:  ---------------------------------------------------------------------------------- 
    } // end of parent process else
    
    return -1; // error!
}

/* TODO: to unblock:

if (sigprocmask(SIG_SETMASK, &prev_all, NULL) < 0) { // unblock
                            perror("sigprocmask");
                            exit(EXIT_FAILURE);
}
*/

/* TODO: to block:

if (sigprocmask(SIG_BLOCK, &mask_all, &prev_all) < 0) { // block
                            perror("sigprocmask");
                            exit(EXIT_FAILURE);
                        }
*/


// The start_c logic: 

            // STEP 2: Create a pipe                                                         ------> pipe(2) 
            // pipe(2) to recieve a startup sunch meassage from a deamon
    
            // STEP 3: Fork a child process to run the daemon command
            
            // STEP 4: Arrange for stdout output of the shild process to be redirected to the appropriate log file:      ------> dup2(2)    ------>  freopen(3)
       
    // STEP 5: Make the child process use execvpe(3) to execute the command registered for daemon          ------> execvpe(3)
    
    // STEP 6: The enviroment passed to execvpe() should be the environment of legion.                    ------> environ(7)
    
    // *  except the value fo the PATH environment variable shoudl be modified: 
    
    // Modify the PATH variable of the environment:                        ------> getenv(3)      ------> putenv(3)     ------> setenv(3)
    
    // by prepending the value of the SERVERS_DIR preprocessor symbol
    
    // separated with ':' of the prepended directory name from existing list
    
    // STEP 7: After the child process has started: // STEP 5: Make the child process use execvpe(3) to execute the command registered for daemon          ------> execvpe(3)
    
    // STEP 6: The enviroment passed to execvpe() should be the environment of legion.                    ------> environ(7)
    
    // *  except the value fo the PATH environment variable shoudl be modified: 
    
    // Modify the PATH variable of the environment:                        ------> getenv(3)      ------> putenv(3)     ------> setenv(3)
    
    // by prepending the value of the SERVERS_DIR preprocessor symbol
    
    // separated with ':' of the prepended directory name from existing list
    
    // * before calling the execvpe() it should use the setgpid() to create and join a new process group        ------> setgpid(2)
    // * To not recieve any signals from terminal
    
    // STEP 8: Should also redirect the output side of the pipe that was created before fork to the file descriptor SYNC_FD
    
    // STEP 9: Daemon executanel writes a one-byte sunch message onto the pipe where it can be read in the parent process
    
// Method that attemts to stop a daemon:    
int stop_c(FILE *in, FILE *out, char* name){
    Daemon *daemon = find_daemon(name);
    
    
    // Meaning the daemon is not found: 
    if (daemon == NULL){
        return -1; // error
    }
    //printf("%s\n", daemon -> status); 
    //if status(daemon) != 'active' ==> error:
    //if ((strcmp(daemon->status, "active") != 0) || (strcmp(daemon->status, "crashed") != 0) || (strcmp(daemon->status, "exited") != 0)){
    
    if ((strcmp(daemon -> status, "active") != 0)) {
    ///printf("%s\n", daemon -> status); 
        return -1; // error --> daemon is not active
    } 
    
    //printf("%s\n", "hereh here here here hreererere x40"); 
    if ((strcmp(daemon->status, "crashed") == 0) || (strcmp(daemon->status, "exited") == 0)){
        set_status(out, daemon, "reset");
        sf_reset(name);
        return 0;
    }  
    
    // set status to 'stopping':
    
    set_status(out, daemon, "stopping");
    sf_stop(name, daemon -> pid);
    // SIGTERM is sent to daemon process:
    if (kill(daemon->pid, SIGTERM) == -1) {
        sf_error("Error: Failed to send SIGTERM signal");
        return -1;
    }    
    
    // Legion then pauses waiting for SIGCHLD signal that indicates that the daemon process has terminated:
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
/*
    // Unblock SIGCHLD temporarily to allow it to be caught:
    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
        perror("Failed to unblock SIGCHLD");
        return -1;
    }
    
    // Pause until a signal is caught:
    int sig;
    while ((sig = sigsuspend(&mask)) == -1 && errno == EINTR);

    // *at this point, SIGCHLD has been caught
    
    // Reset the signal mask to block SIGCHLD again
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        perror("Failed to block SIGCHLD again");
        return -1;
    }
    */
    // Legion then pauses ()            ------> sigsuspend(2):   
    // waiting fir SIGCHILD signal that indicates that the daemon process has terminated    
    // At the same time SIGTERM is sent, a timer is set for CHILD_TIMEOUT seconds
    alarm(CHILD_TIMEOUT);
    
    // if the timer expires before a notification is recieved via SIGCHLD and subsequent waitpid(2)
    // that daemon terminated 
    // => SIGKILL is sent to the daemon process   
    // legion doesnt wait anymore ===> error and return to the prompt   
    
    int status;
    pid_t pid = waitpid(daemon->pid, &status, 0);
    // Waiting:
    // Check if waitpid returned due to SIGCHLD signal:
    if (pid > 0) { 
        // the daemon process has terminated:   
        set_status(out, daemon, "inactive");    
        daemon -> pid = 0; 
        sf_term(name, pid, status);
        
    // the timer has expired before termination: 
    } else if (pid == 0) {  
    // Send SIGKILL signal to the daemon process:  
         if (kill(daemon->pid, SIGKILL) == -1) {
            fprintf(out, "%s","Error: Failed to send SIGKILL signal");
            return -1;
        }   
        else{set_status(out, daemon, "inactive"); sf_kill(name, pid);}
        // Timeout:
        fprintf(out, "Timeout: Daemon process did not terminate within %d seconds\n", CHILD_TIMEOUT);
        return -1; 
    } else {
        // Error in waitpid:
        fprintf(out, "%s","Error: waitpid");
        return -1;
    }   
    return 0; // no errors
} 

// statuses:
/*
WIFEXITED(status): Evaluates to true if the child process terminated normally.
WEXITSTATUS(status): Retrieves the exit status of the child process if it terminated normally.
WIFSIGNALED(status): Evaluates to true if the child process was terminated by a signal.
WTERMSIG(status): Retrieves the number of the signal that terminated the child process.
*/
