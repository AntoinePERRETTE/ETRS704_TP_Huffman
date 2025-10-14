#include <stdint.h>
#include <stdio.h>
#include <stm32f446xx.h>
#include "util.h"
#include "usart.h"


/* Send on USART2 PA2*/
int __io_putchar(int ch){
	USART2_TransmitChar(ch);
	return 0;
}

/* Receive on USART2 PA3 */
int __io_getchar(void){
	return USART2_ReceiveChar();
}

/* For scanf function */
void SCANF_Init(void){
	setvbuf(stdin, NULL, _IONBF, 0);
}

void FPU_Init(void){
	SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));
}
