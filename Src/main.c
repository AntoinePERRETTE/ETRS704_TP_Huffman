#include <stm32f446xx.h>
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "timer.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TAILLE_MAX_COMPRESS 500

// Noeud de l'Arbre
struct noeud{
    uint8_t c;                      // Caractère initial
    uint32_t occurence;             // nombre d'occurrences
    uint32_t code;                  // Code binaires dans l'arbre
    uint32_t tailleCode;            // Nombre de bits du code
    struct noeud *gauche, *droite;  // Lien vers les noeuds suivants
};

void occurence(uint8_t* chaine, uint32_t tab[]) {
    while(*chaine != 0) ++tab[*chaine++];
}

void afficherTabCaractere(uint32_t tab[256]) {
	for(uint8_t i = 0; i < 255; i+=5) {
		printf("[%d : %d] ; ", i, tab[i]);
		printf("[%d : %d] ; ", i+1, tab[i+1]);
		printf("[%d : %d] ; ", i+2, tab[i+2]);
		printf("[%d : %d] ; ", i+3, tab[i+3]);
		printf("[%d : %d]\r\n", i+4, tab[i+4]);
	}
}

void afficherTabArbreHuffman(struct noeud* arbre[256], uint32_t taille) {
    printf("\n[DEBUG] -> Affichage des noeuds générés:\r\n");
    for(uint32_t i = 0; i < taille; i++) {
        if (arbre[i] != NULL) {
            printf("noeud %d : c = %c, occurence = %d, code = %X, tailleCode = %d\r\n", i, arbre[i]->c, arbre[i]->occurence, arbre[i]->code, arbre[i]->tailleCode);
        } else {
            printf("noeud %d n'existe pas\r\n", i);
        }
    }
}

void afficherArbre(struct noeud * ptrNoeud) {
	if(ptrNoeud->droite == NULL && ptrNoeud->gauche == NULL) {
		printf("->[N : %c ; Occ : %d ; code = %X]\0338\n", ptrNoeud->c, ptrNoeud->occurence, ptrNoeud->code);
	} else {
		printf("->[N : %c ; Occ : %d ; code = %X]\0337", ptrNoeud->c, ptrNoeud->occurence, ptrNoeud->code);
		afficherArbre(ptrNoeud->droite);
		afficherArbre(ptrNoeud->gauche);
	}
}

void afficherEnTete(uint8_t *fichier) {
    //Affichage de la taille de l'entête
    printf("[DEBUG] -> l'entete a pour taille : %d\r\n", (int) fichier[0] + (fichier[1] << 8));

    //Affichage de la taille du fichier compressé
    printf("[DEBUG] -> le fichier compresse a pour taille : %d\r\n", (int) fichier[2] + (fichier[3] << 8));

    //Affichage du nombre total de caractère dans le fichier d'origine
    printf("[DEBUG] -> le fichier original contient %d caractere\r\n", (int) fichier[4] + (fichier[5] << 8));

    //Affichage de chaque caractere, de son code et de la taille de ce code
    for(uint16_t i = 6; i < (fichier[0] + (fichier[1] << 8) - 9); i += 9) {
    	printf("[DEBUG] -> Le caractere %c a pour code %X, de taille %d\r\n", fichier[i], fichier[i+1] + (fichier[i+2] << 8) + (fichier[i+3] << 16) + (fichier[i+4] << 24),
    																				fichier[i+5] + (fichier[i+6] << 8) + (fichier[i+7] << 16) + (fichier[i+8] << 24));
    }
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

void afficherCaractereEtCode(struct noeud * racine, uint8_t * texte) {
    struct noeud * ptrNoeud = NULL;
    while(*texte != 0) {
        ptrNoeud = getAddress(racine, *texte);
        if (ptrNoeud != NULL) printf("[DEBUG] -> Le caractere %d à pour code %X\r\n", (uint8_t) *texte, ptrNoeud->code);
        else printf("\r\n[DEBUG] -> Le caractere %d n'est pas dans l'arbre", (uint8_t) *texte);
        texte++;
    }
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

int main(void)
{
	GPIO_Init();
	USART2_Init();
	// Texte non compressé
	// uint8_t texte[] = "aaaabbbccd";
	uint8_t texte[] = "Une banane";

	// Texte compressé
	uint8_t texteCompress[TAILLE_MAX_COMPRESS] = {0};

	// Tableau du nombre d'occurence de chaque caractère
	uint32_t tabCaractere[256] = {0};

	// Nombre de caractère total dans le texte non compréssé
	uint16_t nbrCaractereTotal = strlen((char *) texte);

    // Nombre de caractère total dans le texte compréssé
    uint16_t nbrCaractereCompresseTotal = 0;

	// Nombre de noeuds avec caractère
	uint8_t nbrNoeuds = 0;

	// Arbre de Huffman
	struct noeud* arbreHuffman[256] = {NULL};

    uint8_t fichierFinal[1000] = {0};

    uint8_t fichierReçu[1000] = {0};

	printf("\r\n\n[DEBUG] -> Calcul des occurences\r\n");
	occurence(texte, tabCaractere);

	afficherTabCaractere(tabCaractere);

	nbrNoeuds = creerFeuille(arbreHuffman, tabCaractere);

	if(nbrNoeuds == 0) printf("\n[ERROR] -> Erreur lors de la creation des feuilles\r\n");

	afficherTabArbreHuffman(arbreHuffman, nbrNoeuds);

	printf("\n[DEBUG] -> Tri des noeuds\r\n");
	triArbre(arbreHuffman, nbrNoeuds);

	afficherTabArbreHuffman(arbreHuffman, nbrNoeuds);

	// Référence de la racine de l'arbre
	struct noeud * racine = NULL;

	racine = creerArbre(arbreHuffman, nbrNoeuds);

	if(racine == NULL) printf("\n[ERROR] -> Erreur lors de la creation de l'arbre\r\n");

	printf("\n[DEBUG] -> Creation des code binaires\r\n");
	creerCode(racine, 0, 0);

	printf("\n[DEBUG] -> Affichage de l'arbre\r\n");
	afficherArbre(racine);

	//struct noeud * n = getAddress(racine, 'c');
    afficherCaractereEtCode(racine, texte);

    nbrCaractereCompresseTotal = compresse(texte, texteCompress, racine);

    creerFichier(fichierFinal, texteCompress, racine, nbrCaractereCompresseTotal, nbrCaractereTotal, tabCaractere);

    //Transmission
    for(uint16_t i = 0; i < TAILLE_MAX_COMPRESS; i++) {
    	fichierReçu[i] = fichierFinal[i];
    }

    //Description du fichier reçu
    afficherEnTete(fichierReçu);
    uint16_t tailleDeLEntete = fichierReçu[0] + (fichierReçu[1] << 8);
    uint16_t tailleDuFichierCompresse = fichierReçu[2] + (fichierReçu[3] << 8);
    uint16_t tailleFichierOrigine = fichierReçu[4] + (fichierReçu[5] << 8);

}
