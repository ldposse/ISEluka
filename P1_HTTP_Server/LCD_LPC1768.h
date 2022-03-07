/*
* LCD Interface Header
*/

#ifndef LCD_LPC1768
#define LCD_LPC1768

#include <stdint.h>

extern void LCD_Initialize (void); //Inicializacion del LCD

extern void LCD_Reset (void); //Resetea el LCD

extern void wr_LCD (void); //Escribe en el LCD lo que hay en buffer

extern int EscribeLetra_L1(uint8_t);  //Escribe una letra en buffer

extern int EscribeLetra_L2(uint8_t);  //Escribe una letra en buffer

void EscribeLinea1(char *);   //Escribe una cadena en la linea 1

extern void EscribeLinea2(char *);   //Escribe una cadena en la linea 2

extern void LimpiarBuffer(void);

#endif
