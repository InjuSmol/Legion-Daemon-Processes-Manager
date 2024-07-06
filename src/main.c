/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */

#include <stdlib.h>

#include "legion.h"
//#include <string.h> // TODO: remove this 
//#include "main_helper.h" // TODO: remove this, anyways will be replaced, this file



extern int sf_manual_mode;

int main(int argc, char const *argv[]) {


    if(argc == 1)
	sf_manual_mode = 1;

    sf_init();
    run_cli(stdin, stdout);
    sf_fini();
    return EXIT_SUCCESS;
   
}
// TODO: 

// 1. Enum for statuses 
// 2. Clean all the printf's 
// 3. It is okay to use fprintf and getline and etc. 
// 4. Check the memory leaks with valgrinds
// 5. Signals + sf_methods !
// 6. Check all the exits and returns !!!!
// 7. Terminate all the child proesses bofre quiiting or before cathing quit signal !!!!!!!!

// 7. make sure to free all the structures and all the strucutres, daemons_g, all the Daemon's and etc., parsed_input, 
// 6. Test again 
