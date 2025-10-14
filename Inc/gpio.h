#ifndef GPIO_H_
#define GPIO_H_

void GPIO_Init(void);

void 		hal_SetPin(GPIO_TypeDef* GPIO, uint16_t pin);
void 		hal_ResetPin(GPIO_TypeDef* GPIO, uint16_t pin);
void 		hal_TogglePin(GPIO_TypeDef* GPIO, uint16_t pin);
uint16_t 	hal_ReadPin(GPIO_TypeDef* GPIO, uint16_t pin);

#endif
