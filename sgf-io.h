
#ifndef __SGF_IO__
#define __SGF_IO__


/**********************************************************************
 *
 *  La structure OFILE d�crit un fichier ouvert.
 *
 *  Cette structure comporte des informations sur l'implantation
 *  physique du fichier ainsi que des champs destin�s � d�crire
 *  l'�tat du fichier.
 *
 *********************************************************************/

#define READ_MODE       (0)
#define WRITE_MODE      (1)

struct OFILE            /* "Un fichier ouvert"                  */
    {                   /* ------------------------------------ */
    int   length;       /* taille du fichier (en octets)        */
    int   first;        /* adresse du premier bloc logique      */
    int   last;         /* adresse du dernier bloc logique      */
    int   inode;        /* adresse de l'INODE (descripteur)     */
    int   ptr;          /* n� logique du prochain caract�re     */

    int   mode;         /* READ_MODE ou WRITE_MODE              */
    BLOCK buffer;       /* buffer contenant le bloc courant     */
    };

typedef struct OFILE OFILE;

/**********************************************************************
 *
 *  ROUTINES DE GESTION DES E/S VERS DES FICHIERS OUVERTS (OFILE)
 *
 *********************************************************************/

/************************************************************
 *  Ecrire un caract�re/une cha�ne sur un fichier ouvert en
 *  �criture.
 ************************************************************/

    void sgf_puts (OFILE* f, char *s);
    void sgf_putc (OFILE* f, char  c);

/************************************************************
 *  Lire un caract�re sur un fichier ouvert en lecture.
 *  renvoyer -1 en cas de fin de fichier.
 ************************************************************/

    int sgf_getc (OFILE* f);

/************************************************************
 *  Ouvrir/Fermer/Partager un fichier.
 ************************************************************/

    OFILE* sgf_open  (const char *nom, int mode);
    void   sgf_close (OFILE* f);

/**********************************************************************
 * Initialiser le Syst�me de Gestion de Fichiers.
 *********************************************************************/

    void init_sgf ();

/**********************************************************************
 * Déplacement du pointeur en lecture.
 *********************************************************************/
    
    int sgf_seek (OFILE *file, int pos);
    
#endif
