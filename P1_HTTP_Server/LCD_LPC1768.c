#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include "DRIVER_SPI.h"
#include "SPI_LPC17xx.h"
#include "LCD_LPC1768.h"
#include "Arial12x12.h"
#include <string.h>

#define PORT_0 0
#define PIN_RESET 8
#define PIN_CS 18
#define PIN_A0 6

void retardo (uint32_t); //Genera un retardo de n microsegundos

void wr_data(unsigned char); //Envia un dato al LCD

void wr_cmd( unsigned char ); //Envia un comando al LCD

unsigned char buffer[512];  

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

  static uint32_t posicionL2 = 256;
  static uint32_t posicionL1 = 0;

/*
* Inicialización del LCD
*/
void LCD_Initialize (void){
  SPIdrv->Initialize(0); //Se inicializa la interfaz SPI
  SPIdrv->PowerControl(ARM_POWER_FULL); //Se establece el modo ARM_POWER_FULL
  SPIdrv->Control(ARM_SPI_MODE_MASTER | //Se hablilita 
                      ARM_SPI_CPOL1_CPHA1 |
                      ARM_SPI_MSB_LSB     | //El orden de los bits sera de MSB a LSB
                      ARM_SPI_DATA_BITS(8) //Bits por trama
                      , 20000000); //Velocidad del bus (20 MHz)
  
  GPIO_SetDir(PORT_0, PIN_RESET, GPIO_DIR_OUTPUT); //Pin de reset del LCD
  GPIO_SetDir(PORT_0, PIN_CS, GPIO_DIR_OUTPUT); //Pin de Chip Select del LCD
  GPIO_SetDir(PORT_0, PIN_A0, GPIO_DIR_OUTPUT); //Pin A0 del LCD
  
  GPIO_PinWrite(PORT_0, PIN_RESET, 1); //Se ponen a 1
  GPIO_PinWrite(PORT_0, PIN_CS, 1);
  GPIO_PinWrite(PORT_0, PIN_A0, 1);
  
  GPIO_PinWrite(PORT_0, PIN_RESET, 0); //Generamos el pulso de reset
  retardo (1); //Segun el catalogo del driver del LCD, para una alimentacion de 3.3V, la duracion minima del pulso de reset tiene que ser de 1 us
  GPIO_PinWrite(PORT_0, PIN_RESET, 1);
  retardo(1000); //Esperamos 1 ms antes de realizar mas acciones sobre el LCD
}

/*
* Genera un retardo
*/
void retardo (uint32_t n_microsegundos){
  
  LPC_TIM0->MR0 = (n_microsegundos*25); //Se establece el numero de estados para comparar 
  LPC_TIM0->MCR |= (1 << 2) | (1 << 0); //se resetea y para la cuenta cuando el registro llega al valor de cuenta establecido. Activa el bit de interrupcion
  LPC_TIM0->TCR |= 1 << 1; //Se resetea el timer 
  LPC_TIM0->TCR = 1 << 0; //Se habilita el timer para que empiece a contar
  
  while(!(LPC_TIM0->IR & 1<<0)); //Esperamos a que se active el flag de match en MR0

  LPC_TIM0->IR |= (1 << 0); //Borramos flag
}

/*
* Envia un dato al LCD
*/
void wr_data(unsigned char data){
  GPIO_PinWrite(PORT_0, PIN_CS, 0); //Selecionamos el SSP con el chip select
  GPIO_PinWrite(PORT_0, PIN_A0, 1); //A0 = 1, para indicar que lo que enviamos es un dato
  SPIdrv->Send(&data, sizeof(data)); //Enviamos la direccion del dato, junto a su tamaño en bytes
  GPIO_PinWrite(PORT_0, PIN_CS, 1); //Deseleccionamos el SSP con el chip select
}

/*
* Envía un comando al LCD
*/
void wr_cmd(unsigned char cmd){
  GPIO_PinWrite(PORT_0, PIN_CS, 0); //Selecionamos el SSP con el chip select
  GPIO_PinWrite(PORT_0, PIN_A0, 0); //A0 = 0, para indicar que lo que enviamos es un comando
  SPIdrv->Send(&cmd, sizeof(cmd)); //Enviamos la direccion del comando, junto a su tamaño en bytes
  GPIO_PinWrite(PORT_0, PIN_CS, 1); //Deseleccionamos el SSP con el chip select
}

/*
* Resetea el LCD
*/
void LCD_Reset(void){
  wr_cmd(0xAE); //Display OFF
  wr_cmd(0xA2); //Relacion de tension de polarizacion a 1/9
  wr_cmd(0xA0); //Direccionamiento de la RAM normal
  wr_cmd(0xC8); //Direccion de scan normal
  wr_cmd(0x22); //Ratio de resistencia interna 2
  wr_cmd(0x2F); //Solo se usa la alimentacion interna
  wr_cmd(0x40); //La RAM del display empieza en 0
  wr_cmd(0xAF); //Display ON
  wr_cmd(0x81); //Contraste
  wr_cmd(0x17); //Valor contraste
  wr_cmd(0xA4); //Display al points normal
  wr_cmd(0xA6); //LCD display normal
}

/*
* Escribe lo que hay en el buffer al LCD
*/
void wr_LCD (void) {
  int i;
  wr_cmd(0x00); //4 bits de la parte baja de la direccion a 0
  wr_cmd(0x10); //4 bits de la parte alta de la direccion a 0
  wr_cmd(0xB0); //Pagina 0
  
  for( i = 0; i<128; i++){ 
    wr_data(buffer[i]);
  }
  
  wr_cmd(0x00); //4 bits de la parte baja de la direccion a 0
  wr_cmd(0x10); //4 bits de la parte alta de la direccion a 0
  wr_cmd(0xB1); //Pagina 1
  
  for(i = 128; i<256; i++){
    wr_data(buffer[i]);
  }
  
  wr_cmd(0x00); //4 bits de la parte baja de la direccion a 0
  wr_cmd(0x10); //4 bits de la parte alta de la direccion a 0
  wr_cmd(0xB2); //Pagina 2
  
  for(i = 256; i<384; i++){
    wr_data(buffer[i]);
  }
  
  wr_cmd(0x00); //4 bits de la parte baja de la direccion a 0
  wr_cmd(0x10); //4 bits de la parte alta de la direccion a 0
  wr_cmd(0xB3); //Pagina 3
  
  for(i = 384; i<512; i++){ 
    wr_data(buffer[i]);
  }

}

/*
* Escribe una letra en la linea 1
*/
int EscribeLetra_L1(uint8_t letra){
 
  uint8_t i, valor1, valor2;
  uint16_t comienzo = 0;
  
  comienzo = 25*(letra - ' ');
  
  if((posicionL1 + Arial12x12[comienzo]) < 128 && posicionL1 < 128){ //Para que solo se escriba en la linea 1, y solo si entra la letra en la linea
    for( i = 0; i < 12; i++){
      
      valor1 = Arial12x12[comienzo + i*2 + 1];
      valor2 = Arial12x12[comienzo + i*2 + 2];
      
      buffer[i + posicionL1] = valor1;
      buffer[i+128 + posicionL1] = valor2;
      
    }
  }
  
  posicionL1 = posicionL1 + Arial12x12[comienzo];
  return 0;

}

/*
* Escribe una letra en la linea 2
*/
int EscribeLetra_L2(uint8_t letra){
 
  uint8_t i, valor1, valor2;
  uint16_t comienzo = 0;
  
  comienzo = 25*(letra - ' ');
  
  if((posicionL2 + Arial12x12[comienzo]) < 384 && posicionL2 < 384){ //Para que solo se escriba en la linea 2, y solo si entra la letra en la linea
    for( i = 0; i < 12; i++){
      
      valor1 = Arial12x12[comienzo + i*2 + 1];
      valor2 = Arial12x12[comienzo + i*2 + 2];
      
      buffer[ i + posicionL2] = valor1;
      buffer[i+ 128 + posicionL2] = valor2;
      
    }
  }
  
  posicionL2 = posicionL2 + Arial12x12[comienzo];
  return 0;

}

/*
* Escribe una cadena en la linea 1
*/
void EscribeLinea1(char *cadena){
  static int i;
  posicionL1 = 0;
  for(i = 0; i < strlen(cadena); i++){
    EscribeLetra_L1(cadena[i]);
  }
}

/*
* Escribe una cadena en la linea 2
*/
void EscribeLinea2(char *cadena){
  static int i;
  posicionL2 = 256;
  for(i = 0; i < strlen(cadena); i++){
    EscribeLetra_L2(cadena[i]);
  }
}


/* 
* Borra el buffer
*/
void LimpiarBuffer (void){
  static int i;
  for(i = 0; i < 512; i++){
    buffer[i] = 0;
  }
}
