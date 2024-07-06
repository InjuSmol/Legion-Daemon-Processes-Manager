
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
