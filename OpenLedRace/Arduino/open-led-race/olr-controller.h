#ifndef _OLR_CONTROLLER_LIB_h
#define _OLR_CONTROLLER_LIB_h                   

#ifdef __cplusplus

extern "C"{
#endif

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>

#include "olr-settings.h"


extern int DIGITAL_CTRL[MAX_PLAYERS]; // Global Array containig PINs used for the Digital Controllers (ex: Push Buttons)
//int DIGITAL_CTRL[MAX_PLAYERS]; // Global Array containig PINs used for the Digital Controllers (ex: Push Buttons)


enum ctr_idx {  // Used to access controller by "name" (and not via zero-offset index)
    CTRL_1 = 0, // Ex:  DIGITAL_CTRL[CTRL_2]
    CTRL_2 ,
    CTRL_3 ,
    CTRL_4 
};



#define PIN_VCC_ADC1   6  
#define PIN_VCC_ADC2   7  

enum ctr_type {
    NOT_DEFINED = 0,
    DIGITAL_MODE,
    ANALOG_MODE,
    DEMO_MODE,
};

typedef struct{
    enum ctr_type mode;
    int pin;
    int adc;
    int badc;
    int delta_analog;
    byte flag_sw;
}controller_t;    

void controller_setup( void );

void controller_init( controller_t* ct, enum ctr_type mode, int pin );

byte controller_getStatus( controller_t* ct );

float controller_getSpeed( controller_t* ct );

float controller_getAccel ( void );

bool controller_isActive( int pin );


#ifdef __cplusplus
} // extern "C"
#endif

#endif 
 
