#ifndef _OLR_PARAM_LIB_h
#define _OLR_PARAM_LIB_h                   

#ifdef __cplusplus

extern "C"{
#endif

#include "Arduino.h"
#include <stdint.h>
#include <stdbool.h>

#include "olr-settings.h"

// Default values loaded on "D" command received (Serial Protocol) 
//////////////////////////////////////////////////////////////////
#define MAXLED          300 
#define BOXLEN          60 
#define NUMLAP          5 
#define BATTERY_MODE    false
#define AUTOSTART_MODE  false
#define BOX_ALWAYS_ON   false
#define SLOPE_ALWAYS_ON false
#define PLAYER_3        false
#define PLAYER_4        false
#define DEMO_MODE_ST    false
//////////////////////////////////////////////////////////////////


// Position (bit) into the cfgparam.option byte for On|Off settings
// Used in param_option_set()/param_option_is_active() calls
enum cfgparam_option_bit {
  BATTERY_MODE_OPTION   = 0,
  AUTOSTART_MODE_OPTION = 1,
  BOX_MODE_OPTION       = 2,
  SLOPE_MODE_OPTION     = 3,
  PLAYER_3_OPTION       = 4,
  PLAYER_4_OPTION       = 5,
  DEMO_MODE_OPTION      = 6,
  NOT_USED_7_OPTION     = 7,
};

enum cfgpar {
  CFGPARAM_VER = 7, // Change this value (+=1) every time the [cfgparam] struct is modified
                    // This will force an update with the new [struct] to the settings 
                    // stored in EEPROM with an old (invalid) struct
  LEN_UID = 16,
};


typedef struct cfgrace{
    bool startline;   // Standalone mode: Always 1
    uint8_t  nlap;
    uint8_t  nrepeat; // Standalone mode: Always 1
    bool finishline;  // Standalone mode: Always 1
} cfgrace_t;

struct cfgbattery{   // added in ver 0.9.7
  uint8_t delta;     // unsigned char value [1-254] / will be divided by 100 [0.01-2.54]
  uint8_t min;       // Battery charge does not goes below this "min" percentage
  uint8_t speed_boost_scaler;
} ;

struct cfgtrack  {
  int nled_total;
  int nled_main;
  int nled_aux;
  int init_aux;
  int box_len;  // used to hold the Box Length if the default is changed.
                // it's not possible to implicitly store it in nled_main,nled_aux
                // because, if these are different to the default, box gets always activated
                // (the software does not chek "box_isactive" to draw car position)
  float kf;
  float kg;
  
};

// ramp centred in LED 100 with 10 led fordward and 10 backguard
struct cfgramp  {
  int init;
  int center;
  int end;
  uint8_t high;
};

struct brdinfo {
  char uid[LEN_UID + 1];
};

struct cfgparam {
    uint8_t ver;      // Version of this [cfgparam] struct 
    uint8_t option;   // Bit-mapped byte to store 'active' on|off for options (Battery, AutoStart, BoxalwaysOn, etc)
    //struct cfgrace    race;  // added in ver 0.9.d
    cfgrace_t   race;  // added in ver 0.9.d
    struct cfgbattery battery;
    struct cfgtrack   track;
    struct cfgramp    ramp;
    struct brdinfo    info;
};


void param_setdefault( struct cfgparam* cfg );
void param_option_set( struct cfgparam* cfg, uint8_t option, boolean value );
boolean param_option_is_active( struct cfgparam* cfg, uint8_t option);

#ifdef __cplusplus
} // extern "C"
#endif

#endif 
 
