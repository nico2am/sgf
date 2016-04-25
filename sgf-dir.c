
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "sgf-disk.h"
#include "sgf-fat.h"
#include "sgf-data.h"
#include "sgf-dir.h"


static int directory_first_block = -1;

/**********************************************************************
 rechercher et renvoyer l'adresse du descripteur d'un fichier.
 Cette fonction renvoie -1 en cas d'erreur.
 *********************************************************************/

int find_inode(const char* name) {
    int adr;
    TBLOCK b;
    int j;

    if (directory_first_block == -1) {
        read_block(ADR_BLOCK_DEF, & b.data);
        directory_first_block = b.super.adr_dir;
    }

    adr = directory_first_block;

    while (adr != FAT_EOF) {
        read_block(adr, &b.data);
        for (j = 0; j < BLOCK_DIR_SIZE; j++)
            if (b.dir[j].inode > 0)
                if (strcmp(b.dir[j].name, name) == 0)
                    return (b.dir[j].inode);
        adr = get_fat(adr);
    }

    return (-1);
}

/**********************************************************************
 Ajouter un couple <name,inode> au r�pertoire. Si un couple existe d�j�,
 la fonction renvoie l'adresse du descripteur et -1 dans le cas
 contraire.
 *********************************************************************/

int add_inode(const char* name, int inode) {
    int oldinode, padr, adr, nadr;
    int j, nj;
    TBLOCK b;

    if ((strlen(name) + 1) > LONG_FILENAME) {
        return (-1);
    }

    if (directory_first_block == -1) {
        read_block(ADR_BLOCK_DEF, & b.data);
        directory_first_block = b.super.adr_dir;
    }

    adr = directory_first_block;

    nadr = nj = -1;
    while (adr != FAT_EOF) {
        read_block(adr, &b.data);
        for (j = 0; j < BLOCK_DIR_SIZE; j++)
            if (b.dir[j].inode > 0)
                if (strcmp(b.dir[j].name, name) == 0) {
                    oldinode = b.dir[j].inode;
                    b.dir[j].inode = inode;
                    write_block(adr, & b.data);
                    return (oldinode);
                } else;
            else nadr = adr, nj = j;
        padr = adr;
        adr = get_fat(adr);
    }

    if (nadr != -1) {
        read_block(nadr, & b.data);
        b.dir[nj].inode = inode;
        strcpy(b.dir[nj].name, name);
        write_block(nadr, & b.data);
        return (-1);
    }

    /** Allouer un nouveau bloc pour le r�pertoire **/
    adr = alloc_block();
    if (adr < 0) return (-1);

    /** Initialiser ce nouveau bloc **/
    for (j = 0; j < BLOCK_DIR_SIZE; j++)
        b.dir[j].inode = 0;

    /** Utiliser la 1ere entree de ce bloc **/
    b.dir[0].inode = inode;
    strcpy(b.dir[0].name, name);
    write_block(adr, & b.data);

    /** Mettre � jour la FAT **/
    set_fat(adr, FAT_EOF);
    set_fat(padr, adr);

    return (-1);
}

/**********************************************************************
 Effacer un couple <name,inode> au r�pertoire.
 *********************************************************************/

void delete_inode(const char* name) {
    int adr;
    TBLOCK b;
    int j;

    if (directory_first_block == -1) {
        read_block(ADR_BLOCK_DEF, & b.data);
        directory_first_block = b.super.adr_dir;
    }

    adr = directory_first_block;

    while (adr != FAT_EOF) {
        read_block(adr, & b.data);
        for (j = 0; j < BLOCK_DIR_SIZE; j++)
            if (b.dir[j].inode > 0)
                if (strcmp(b.dir[j].name, name) == 0) {
                    b.dir[j].inode = 0;
                    write_block(adr, & b.data);
                    return;
                }
        adr = get_fat(adr);
    }
}

/**********************************************************************
 Formater le disque et cr�er un r�pertoire vide.
 *********************************************************************/

void create_empty_directory() {
    int adr_repertoire;
    TBLOCK b;
    int j;

    /* lire le super bloc */
    read_block(0, &b.data);
    directory_first_block = adr_repertoire = b.super.adr_dir;

    /* vider le 1er bloc du r�pertoire et le sauver */
    for (j = 0; j < BLOCK_DIR_SIZE; j++) b.dir[j].inode = 0;
    write_block(adr_repertoire, & b.data);

    printf("create empty directory (block %d)\n", directory_first_block);
}

/**********************************************************************
 Lister les fichiers du r�pertoire avec leur taille.
 *********************************************************************/

void list_directory(void) {
    TBLOCK dir, file;
    int inode;
    
    read_block(0, &dir.data);
    
    printf("racine:\n");
    
    read_block(dir.super.adr_dir, &dir.data);
    
    for(int i = 0; i < BLOCK_DIR_SIZE; i++) {
        if(dir.dir[i].inode == 0)
            continue;
        
        inode = find_inode(dir.dir[i].name);
        if(inode < 0) panic("Erreur d'adressage");
        
        read_block(inode, &file.data);
        
        printf("\t%s\t%d octets\n", dir.dir[i].name, file.inode.length);
    }
    
}


