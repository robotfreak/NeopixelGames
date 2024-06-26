#include "olr-controller.h"
enum {
  DELTA_ANALOG = 5,
};

int DIGITAL_CTRL[MAX_PLAYERS];
 
static float const ACEL = 0.2;

void controller_setup( void ) { 

  /***
  if( DIGITAL_MODE == false ){
    pinMode(PIN_VCC_ADC1, OUTPUT);
    pinMode(PIN_VCC_ADC2, OUTPUT);
    digitalWrite(PIN_VCC_ADC1, HIGH);
    digitalWrite(PIN_VCC_ADC2, HIGH);
  }
  ***/
  
  DIGITAL_CTRL[CTRL_1]= DIG_CTRL_1_PIN;
  DIGITAL_CTRL[CTRL_2]= DIG_CTRL_2_PIN;
  DIGITAL_CTRL[CTRL_3]= DIG_CTRL_3_PIN;
  DIGITAL_CTRL[CTRL_4]= DIG_CTRL_4_PIN;
  
  pinMode( DIG_CTRL_1_PIN, INPUT_PULLUP); //pull up in adc
  pinMode( DIG_CTRL_2_PIN, INPUT_PULLUP);
  pinMode( DIG_CTRL_3_PIN, INPUT_PULLUP);
  pinMode( DIG_CTRL_4_PIN, INPUT_PULLUP);
}

void controller_init( controller_t* ct, enum ctr_type mode, int pin ) {
  ct->mode = mode;
  ct->pin = pin;
  ct->delta_analog = DELTA_ANALOG;
}


byte controller_getStatus( controller_t* ct ) {

  if( ct->mode == DIGITAL_MODE ){
    return digitalRead( ct->pin );
  }
  else if( ct->mode == ANALOG_MODE ){
    ct->adc = analogRead( ct->pin );
    if( abs( ct->badc - ct->adc ) > ct->delta_analog ){
      ct->badc = ct->adc;
      return 1;
    }
    ct->badc = ct->adc;
  }
  else if( ct->mode == DEMO_MODE ){
    ct->adc++;
    int rnd = rand();
    long threshold = 1800 - rnd;
    if( ct->adc >= (threshold)){
      ct->adc = 0;
      return 1;
    }
  }
  return 0;

}


float controller_getSpeed( controller_t* ct) {
    float speed = 0.0;
    if ( (ct->flag_sw == 1 ) && (controller_getStatus( ct ) == 0) ) {
        ct->flag_sw = 0;
        speed = ACEL;
    }

    if ( (ct->flag_sw == 0 ) && (controller_getStatus( ct ) == 1 ) ) {
        ct->flag_sw = 1;
    }
    return speed;
}


float controller_getAccel ( void ) {
    return ACEL;
}

bool controller_isActive( int pin ) {
    return !digitalRead( pin );
}
