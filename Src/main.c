#include <stm32f446xx.h>
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "timer.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

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
    printf("\n[DEBUG] -> Affichage des noeud générés:\r\n");
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
		printf("->[Noeud : %c ; occurence : %d]\r\n", ptrNoeud->c, ptrNoeud->occurence);
	} else {
		printf("->[Noeud : %c ; occurence : %d]\0337->[Noeud : %c ; occurence : %d]\0338\n", ptrNoeud->c, ptrNoeud->occurence, ptrNoeud->droite->c, ptrNoeud->droite->occurence);
		afficherArbre(ptrNoeud->gauche);
	}
}

uint8_t creerFeuille(struct noeud * arbre[256], uint32_t tab[256]) {
	uint8_t j = 0;
    struct noeud * ptrNoeud = NULL;
    for(uint8_t i = 0; i < 255; i++) {
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

uint8_t comparerNoeud(struct noeud * n1, struct noeud * n2) {
	if (n1->occurence < n2->occurence) return -1;
	else if (n1->occurence > n2->occurence) return 1;
	else return 0;
}

void triArbre(struct noeud * arbre[256], uint32_t taille) {
	qsort((void *) arbre, taille, sizeof(struct noeud *), comparerNoeud);
}

int main(void)
{
	GPIO_Init();
	SYSTICK_Init();
	USART2_Init();

	// Texte non compressé
	uint8_t texte[] = "aaaabbbccd";

	// Texte compressé
	//uint8_t texteCompress[TAILLE_MAX_COMPRESS];

	// Tableau du nombre d'occurence de chaque caractère
	uint32_t tabCaractere[256] = {0};

	// Nombre de caractère total dans le texte non compréssé
	//uint32_t nbrCaractereTotal = 0;

	// Nombre de caractère différent dans le texte non compréssé
	//uint32_t nbrCaractereDifferent = 0;

	// Nombre de noeuds
	uint8_t nbrNoeuds = 0;

	// Arbre de Huffman
	struct noeud* arbreHuffman[256] = {NULL};

	printf("\r\n\n[DEBUG] -> Calcul des occurences\r\n");
	occurence(texte, tabCaractere);

	afficherTabCaractere(tabCaractere);

	nbrNoeuds = creerFeuille(arbreHuffman, tabCaractere);

	if(nbrNoeuds == 0) printf("\n[ERROR] -> Erreur lors de la création des feuilles\r\n");

	afficherTabArbreHuffman(arbreHuffman, nbrNoeuds);

	printf("\n[DEBUG] -> Tri des noeuds\r\n");
	triArbre(arbreHuffman, nbrNoeuds);

	afficherTabArbreHuffman(arbreHuffman, nbrNoeuds);

	// Référence de la racine de l'arbre
	struct noeud * racine = NULL;

	racine = creerArbre(arbreHuffman, nbrNoeuds);

	if(racine == NULL) printf("\n[ERROR] -> Erreur lors de la création de l'arbre\r\n");

	printf("\n[DEBUG] -> Affichage de l'arbre\r\n");
	afficherArbre(racine);
}
