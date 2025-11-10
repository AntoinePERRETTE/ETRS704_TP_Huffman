#include "huffman.h"

void occurence(uint8_t* chaine, uint32_t tab[]) {
    while(*chaine != 0) ++tab[*chaine++];
}

uint8_t creerFeuille(struct noeud * arbre[256], uint32_t tab[256]) {
	uint8_t j = 0;
    struct noeud * ptrNoeud = NULL;
    for(uint16_t i = 0; i < 256; i++) {
        if (tab[i] != 0) {
            ptrNoeud = (struct noeud *) malloc(sizeof(struct noeud));

            if(ptrNoeud == NULL) return 0;

            ptrNoeud->c = i;
            ptrNoeud->occurence = tab[i];
            ptrNoeud->code = 0;
            ptrNoeud->tailleCode = 0;
            ptrNoeud->gauche = NULL;
            ptrNoeud->droite = NULL;

            arbre[j] = ptrNoeud;
            j++;
        }
    }
    return j;
}

struct noeud * creerArbre(struct noeud * arbre[256], uint32_t taille){
	struct noeud * racine = NULL;
	for(uint8_t i = 0; i < taille-1; i++) {
		racine = (struct noeud *) malloc(sizeof(struct noeud));

		if(racine == NULL) return racine;

		racine->c = '!';
		racine->occurence = arbre[i]->occurence + arbre[i+1]->occurence;
		racine->code = 0;
		racine->tailleCode = 0;
		racine->gauche = arbre[i];
		racine->droite = arbre[i+1];
		arbre[i+1] = racine;
	}
	return racine;
}

void creerCode(struct noeud * ptrNoeud, uint32_t code, uint32_t taille) {
	if(ptrNoeud->droite == NULL && ptrNoeud->gauche == NULL) {
		ptrNoeud->tailleCode = taille;
		ptrNoeud->code = code;
		printf("%c \t code : %X \t taille : %d \r\n", ptrNoeud->c, ptrNoeud->code, ptrNoeud->tailleCode);
	} else {
		//On va a gauche (on injecte un 0 à droite dans le code)
		creerCode(ptrNoeud->gauche, code << 1, taille + 1);
		//On va a droite (on injecte un 1 à droite)
		creerCode(ptrNoeud->droite, (code << 1) + 1, taille + 1);
	}
}

void triArbre(struct noeud * arbre[256], uint32_t taille) {
	struct noeud * n = NULL;

	for(uint8_t i = 0; i < taille-1; i++) {
		for(uint8_t j = 0; j < taille-1; j++) {
			if(arbre[j]->occurence > arbre[j+1]->occurence) {
				n = arbre[j];
				arbre[j] = arbre[j+1];
				arbre[j+1] = n;
			}
		}
	}
}

struct noeud * getAddress(struct noeud * racine, uint8_t caractere) {
    struct noeud * ptrNoeud = NULL;
	if(racine->c == caractere) {
        return racine;
    } else if(racine->droite != NULL && (ptrNoeud = getAddress(racine->droite, caractere)) != NULL) {
        return ptrNoeud;
    } else if(racine->gauche != NULL && (ptrNoeud = getAddress(racine->gauche, caractere)) != NULL) {
        return ptrNoeud;
    }
    return NULL;
}

uint32_t compresse(uint8_t * texte, uint8_t texteCompress[TAILLE_MAX_COMPRESS], struct noeud * racine) {
    struct noeud * ptrNoeud = NULL;
    uint8_t symbole = 0;
    uint8_t tailleSymbole = 0;
    uint32_t index = 0;
    uint32_t tailleRestante = 0;
    while(*texte != 0 && index < TAILLE_MAX_COMPRESS) {
        ptrNoeud = getAddress(racine, *texte);

        tailleRestante = 8 - (uint32_t) tailleSymbole;

        if(ptrNoeud->tailleCode <= tailleRestante) {
            symbole <<= ptrNoeud->tailleCode;
            symbole |= (uint8_t) ptrNoeud->code;
            tailleSymbole += ptrNoeud->tailleCode;
        } else {
            symbole <<= tailleRestante;
            symbole |= (uint8_t) ptrNoeud->code >> (-tailleRestante - ptrNoeud->tailleCode);

            texteCompress[index] = symbole;
            tailleSymbole = 0;
            symbole = 0;
            index++;

            symbole |= (uint8_t) ptrNoeud->code;
            tailleSymbole += ptrNoeud->tailleCode - tailleRestante;
        }

        texte++;
    }
    symbole <<= 8 - tailleSymbole;
    texteCompress[index] = symbole;

    return index+1;
}

void creerFichier(uint8_t fichier[], uint8_t texteCompress[], struct noeud * racine, uint16_t tailleFichierCompresse, uint16_t nbrCaractereTotal, uint32_t tab[256]) {
   fichier[2] = tailleFichierCompresse & 0xFF;
   fichier[3] = (tailleFichierCompresse & 0xFF00) >> 8;

   fichier[4] = nbrCaractereTotal & 0x00FF;
   fichier[5] = (nbrCaractereTotal & 0xFF00) >> 8;

   uint16_t index = 6;
   struct noeud * ptrNoeud = NULL;

   for(uint16_t i = 0; i < 256; i++) {
       if(tab[i] != 0) {
           ptrNoeud = getAddress(racine, i);
           fichier[index] = i;

           fichier[++index] = (ptrNoeud->code & 0x000000FF);
           fichier[++index] = (ptrNoeud->code & 0x0000FF00) >> 8;
           fichier[++index] = (ptrNoeud->code & 0x00FF0000) >> 16;
           fichier[++index] = (ptrNoeud->code & 0xFF00000) >> 24;

           fichier[++index] = (ptrNoeud->tailleCode & 0x000000FF);
           fichier[++index] = (ptrNoeud->tailleCode & 0x0000FF00) >> 8;
           fichier[++index] = (ptrNoeud->tailleCode & 0x00FF0000) >> 16;
           fichier[++index] = (ptrNoeud->tailleCode & 0xFF000000) >> 24;

           index++;
       }
   }

   for(uint16_t i = index; (i - index) < tailleFichierCompresse; i++) {
       fichier[i] = texteCompress[i - index];
   }
   index += tailleFichierCompresse;

   fichier[0] = index & 0x00FF;
   fichier[1] = (index & 0xFF00) >> 8;
}
