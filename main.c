
/*
 **  main.c
 **
 **  Utilisation des fonctions du mini SGF.
 **
 **  04/04/2007
 */

#include <stdio.h>
#include <stdlib.h>

#include "sgf-disk.h"
#include "sgf-fat.h"
#include "sgf-dir.h"
#include "sgf-io.h"

int main() {
    OFILE* file;
    int c;

    init_sgf();

    printf("\nListing du disque\n\n");
    list_directory();

    file = sgf_open("essai.txt", READ_MODE);
    
    while ((c = sgf_getc(file)) > 0) {
        //if(sgf_seek(file, 8) == -1)
            //break;   
          
        putchar(c);
    }
    
    //sgf_close(file);

    return (EXIT_SUCCESS);
}

