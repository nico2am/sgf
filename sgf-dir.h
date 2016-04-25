
#ifndef __SGF_REP_H__
#define __SGF_REP_H__


/**********************************************************************
 Rechercher l'adresse d'un inode � partir d'un nom de fichier.
 cette fonction renvoie -1 en cas d'erreur.
 *********************************************************************/

int find_inode (const char* nom);

/**********************************************************************
 Ajouter un couple <nom,inode> au r�pertoire. Si il existe d�j� un
 couple <nom,inode'> la fonction renvoie inode' sinon elle renvoie -1.
 *********************************************************************/

int add_inode (const char* nom, int desc);

/**********************************************************************
 Effacer un couple <nom,desc> du r�pertoire.
 *********************************************************************/

void delete_inode (const char* nom);

/**********************************************************************
 Formater un disque et cr�er un r�pertoire vide.
 *********************************************************************/

void create_empty_directory (void);

/**********************************************************************
 Lister les fichiers du r�pertoire avec leur taille.
 *********************************************************************/

void list_directory (void);


#endif
