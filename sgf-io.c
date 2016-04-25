
/*
 **  sgf-io.c
 **
 **  fonctions de lecture/�criture (de caract�res et de blocs)
 **  dans un fichier ouvert.
 **
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sgf-disk.h"
#include "sgf-data.h"
#include "sgf-fat.h"
#include "sgf-dir.h"
#include "sgf-io.h"

/**********************************************************************
 *
 *  FONCTIONS DE LECTURE DANS UN FICHIER
 *
 *********************************************************************/

/**********************************************************************
 Lire dans le "buffer" le bloc logique "nubloc" dans le fichier
 ouvert "file".
 
 ATTENTION: Faites en sorte de ne pas recommencer le cha�nage �
            partir du bloc 0 si cela n'est pas utile. Pour �viter ce
            parcours vous pouvez ajouter un champ � la structure OFILE
            qui vous donne l'adresse physique du bloc courant.
 *********************************************************************/

void sgf_read_bloc(OFILE* file, int nubloc) {
    int block;

    assert(file->mode == READ_MODE);

    block = file->first;

    assert(block != FAT_EOF);

    while (nubloc-- > 0) {
        block = get_fat(block);
        assert(block != FAT_EOF);
    }

    read_block(block, &file->buffer);
}

/**********************************************************************
 Lire un caract�re dans un fichier ouvert. Cette fonction renvoie
 -1 si elle trouve la fin du fichier.
 *********************************************************************/

int sgf_getc(OFILE* file) {
    int c;

    assert(file->mode == READ_MODE);

    /* d�tecter la fin de fichier */
    if (file->ptr >= file->length)
        return (-1);

    /* si le buffer est vide, le remplir */
    if ((file->ptr % BLOCK_SIZE) == 0) {
        sgf_read_bloc(file, file->ptr / BLOCK_SIZE);
    }

    c = file->buffer[ (file->ptr % BLOCK_SIZE) ];
    file->ptr++;
    return (c);
}



/**********************************************************************
 *
 *  FONCTIONS D'ECRITURE DANS UN FICHIER
 *
 *********************************************************************/

/**********************************************************************
 Ajouter le bloc contenu dans le tampon au fichier ouvert d�crit
 par "f".
 *********************************************************************/

void sgf_append_block(OFILE* f) {
    
    if(f->mode == APPEND_MODE) {
        //printf("coucou\n");
        write_block(f->last, &f->buffer);
        
        f->mode = WRITE_MODE;
        
        return;
    }
    
    TBLOCK b;
    int adr;

    //panic("%s: ligne %d: fonction non terminee", __FILE__, __LINE__);

    adr = alloc_block();
    assert(adr >= 0);

    write_block(adr, &f->buffer);
    set_fat(adr, FAT_EOF);

    if (f->first == FAT_EOF) {
        f->first = adr;
        f->last = adr;
    } else {
        set_fat(f->last, adr);
        f->last = adr;
    }

    f->length = f->ptr;

    b.inode.length = f->length;
    b.inode.first = f->first;
    b.inode.last = f->last;

    write_block(f->inode, &b.data);
}

/**********************************************************************
 Ecrire le caract�re "c" dans le fichier ouvert d�crit par "file".
 *********************************************************************/

void sgf_putc(OFILE* file, char c) {
    assert(file->mode == WRITE_MODE || file->mode == APPEND_MODE);

    //panic("%s: ligne %d: fonction non terminee", __FILE__, __LINE__);


    file->buffer[file->ptr % BLOCK_SIZE] = c;
    file->ptr++;

    if ((file->ptr % BLOCK_SIZE) == 0)
        sgf_append_block(file);
}

/**********************************************************************
 �crire la cha�ne de caract�re "s" dans un fichier ouvert en �criture
 d�crit par "file".
 *********************************************************************/

void sgf_puts(OFILE* file, char* s) {
    assert(file->mode == WRITE_MODE || file->mode == APPEND_MODE);

    for (; (*s != '\0'); s++) {
        sgf_putc(file, *s);
    }
}



/**********************************************************************
 *
 *  FONCTIONS D'OUVERTURE, DE FERMETURE ET DE DESTRUCTION.
 *
 *********************************************************************/

/************************************************************
 D�truire un fichier.
 ************************************************************/

void sgf_remove(int adr_inode) {
    TBLOCK b;
    int adr;

    //printf("%s: ligne %d: fonction non terminee", __FILE__, __LINE__);

    read_block(adr_inode, &b.data);

    adr = b.inode.first;

    while (adr != FAT_EOF) {
        int adr_tmp = get_fat(adr);
        set_fat(adr, FAT_FREE);
        adr = adr_tmp;
    }

    set_fat(adr_inode, FAT_FREE);

    int counter = 0;
    for (int i = 0; i < get_disk_size(); i++) {
        if (get_fat(i) == FAT_FREE);
        counter++;
    }

    printf("Nombre de block libres: %d\n", counter);
}

/************************************************************
 Ouvrir un fichier en �criture seulement (NULL si �chec).
 ************************************************************/

static OFILE* sgf_open_write(const char* nom) {
    int inode, oldinode;
    OFILE* file;
    TBLOCK b;

    /* Rechercher un bloc libre sur disque */
    inode = alloc_block();
    assert(inode >= 0);

    /* Allouer une structure OFILE */
    file = malloc(sizeof (struct OFILE));
    if (file == NULL) return (NULL);

    /* pr�parer un inode vers un fichier vide */
    b.inode.length = 0;
    b.inode.first = FAT_EOF;
    b.inode.last = FAT_EOF;

    /* sauver ce inode */
    write_block(inode, &b.data);
    set_fat(inode, FAT_INODE);

    /* mettre a jour le repertoire */
    oldinode = add_inode(nom, inode);
    if (oldinode > 0) sgf_remove(oldinode);

    file->length = 0;
    file->first = FAT_EOF;
    file->last = FAT_EOF;
    file->inode = inode;
    file->mode = WRITE_MODE;
    file->ptr = 0;

    return (file);
}

/************************************************************
 Ouvrir un fichier en lecture seulement (NULL si �chec).
 ************************************************************/

static OFILE* sgf_open_read(const char* nom) {
    int inode;
    OFILE* file;
    TBLOCK b;

    /* Chercher le fichier dans le r�pertoire */
    inode = find_inode(nom);
    if (inode < 0) return (NULL);

    /* lire le inode */
    read_block(inode, &b.data);

    /* Allouer une structure OFILE */
    file = malloc(sizeof (struct OFILE));
    if (file == NULL) return (NULL);

    file->length = b.inode.length;
    file->first = b.inode.first;
    file->last = b.inode.last;
    file->inode = inode;
    file->mode = READ_MODE;
    file->ptr = 0;

    return (file);
}

/************************************************************
 Ouvrir un fichier en ajout (NULL si échec).
 ************************************************************/


static OFILE* sgf_open_append(const char* nom) {
    int inode;
    OFILE* file;
    TBLOCK b;

    /* Chercher le fichier dans le répertoire */
    inode = find_inode(nom);
    if (inode < 0) return (NULL);

    /* lire le inode */
    read_block(inode, &b.data);

    /* Allouer une structure OFILE */
    file = malloc(sizeof (struct OFILE));
    if (file == NULL) return (NULL);

    file->length = b.inode.length;
    file->first = b.inode.first;
    file->last = b.inode.last;
    file->inode = inode;
    file->ptr = b.inode.length;

    if ((file->length % BLOCK_SIZE) != 0) {
        read_block(file->last, &file->buffer);
        file->mode = APPEND_MODE;
    } else {
        file->mode = WRITE_MODE;
    }

    return (file);
}

/************************************************************
 Ouvrir un fichier (NULL si �chec).
 ************************************************************/

OFILE* sgf_open(const char* name, int mode) {
    switch (mode) {
        case READ_MODE: return sgf_open_read(name);
        case WRITE_MODE: return sgf_open_write(name);
        case APPEND_MODE: return sgf_open_append(name);
        default: return (NULL);
    }
}

/************************************************************
 Fermer un fichier ouvert.
 ************************************************************/

void sgf_close(OFILE* file) {
    //panic("%s: ligne %d: fonction non terminee", __FILE__, __LINE__);
    if (file->mode == WRITE_MODE || file->mode == APPEND_MODE)
        if (file->ptr > 0)
            sgf_append_block(file);
}

/**********************************************************************
 initialiser le SGF
 *********************************************************************/

void init_sgf(void) {
    init_sgf_disk();
    init_sgf_fat();
}

/**********************************************************************
 * Déplacement du pointeur en lecture.
 *********************************************************************/

int sgf_seek(OFILE *file, int pos) {
    if (pos % BLOCK_SIZE != 0 && file->ptr / BLOCK_SIZE != (file->ptr + pos) / BLOCK_SIZE)
        sgf_read_bloc(file, pos / BLOCK_SIZE);

    if (pos >= file->length)
        return -1;

    file->ptr += pos;

    return 0;

}






