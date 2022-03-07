
#ifndef LED_LPC1768_H
#define LED_LPC1768_H

#include <stdint.h>

extern int32_t  LED_Initialize   (void);
extern int32_t  LED_Uninitialize (void);
extern int32_t  LED_On           (uint32_t num);
extern int32_t  LED_Off          (uint32_t num);
extern int32_t  LED_SetOut       (uint32_t val);
extern uint32_t LED_GetCount     (void);

#endif /*LED_LPC1768_H*/
