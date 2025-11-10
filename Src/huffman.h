#ifndef HUFFMAN_H
#define HUFFMAN_H
void occurence(uint8_t* chaine, uint32_t tab[]);

uint8_t creerFeuille(struct noeud * arbre[256], uint32_t tab[256]);

struct noeud * creerArbre(struct noeud * arbre[256], uint32_t taille);

void creerCode(struct noeud * ptrNoeud, uint32_t code, uint32_t taille);

void triArbre(struct noeud * arbre[256], uint32_t taille);

struct noeud * getAddress(struct noeud * racine, uint8_t caractere);

uint32_t compresse(uint8_t * texte, uint8_t texteCompress[TAILLE_MAX_COMPRESS], struct noeud * racine);

void creerFichier(uint8_t fichier[], uint8_t texteCompress[], struct noeud * racine, uint16_t tailleFichierCompresse, uint16_t nbrCaractereTotal, uint32_t tab[256]);
#endif
