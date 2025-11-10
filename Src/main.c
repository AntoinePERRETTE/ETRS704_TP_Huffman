#include <stm32f446xx.h>
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "timer.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "huffman.h"

#define TAILLE_MAX_COMPRESS 500

// Noeud de l'Arbre
struct noeud{
    uint8_t c;                      // Caractère initial
    uint32_t occurence;             // nombre d'occurrences
    uint32_t code;                  // Code binaires dans l'arbre
    uint32_t tailleCode;            // Nombre de bits du code
    struct noeud *gauche, *droite;  // Lien vers les noeuds suivants
};

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

void afficherCaractereEtCode(struct noeud * racine, uint8_t * texte) {
    struct noeud * ptrNoeud = NULL;
    while(*texte != 0) {
        ptrNoeud = getAddress(racine, *texte);
        if (ptrNoeud != NULL) printf("[DEBUG] -> Le caractere %d à pour code %X\r\n", (uint8_t) *texte, ptrNoeud->code);
        else printf("\r\n[DEBUG] -> Le caractere %d n'est pas dans l'arbre", (uint8_t) *texte);
        texte++;
    }
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
