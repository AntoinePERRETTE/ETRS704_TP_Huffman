#include <stm32g031xx.h>
#include "main.h"
#include "gpio.h"
#include "timer.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int __io_putchar(int ch) {
	USART2->DR = ch;
	while( (USART2->SR & USART_SR_TXE)==0 );
}

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

void afficherTabArbreHuffman(struct noeud* arbre[256], uint32_t taille) {
    printf("[DEBUG] -> Affichage des noeud générés:\n");
    for(uint32_t i = 0; i < taille; i++) {
        if (arbre[i] != NULL) {
            printf("noeud %d : c = %c, occurence = %d, code = %X, tailleCode = %d\n", i, arbre[i]->c, arbre[i]->occurence, arbre[i]->code, arbre[i]->tailleCode);
        } else {
            printf("noeud %d n'existe pas\n", i);
        }
    }
}

void creerFeuille(struct noeud * arbre[256], uint32_t tab[256]) {
    struct noeud * n;
    for(uint8_t i = 0; i < 255; i++) {
        if (tab[i] != 0) {
            n = (struct noeud *) malloc(sizeof(struct noeud));
            n->c = i;
            n->occurence = tab[i];
            n->code = 0;
            n->tailleCode = 0;
            n->gauche = NULL;
            n->droite = NULL;

            *arbre = n;
            arbre++;
        }
    }
}

int main(void)
{
	GPIO_Init();
	SYSTICK_Init();

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

	// Arbre de Huffman
	struct noeud* arbreHuffman[256] = {(void *) 0};

	occurence(texte, tabCaractere);
	creerFeuille(arbreHuffman, tabCaractere);
	afficherTabArbreHuffman(arbreHuffman, 6);
}
