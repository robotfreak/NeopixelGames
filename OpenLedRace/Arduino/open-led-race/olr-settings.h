#ifndef _OLR_COMMON__h
#define _OLR_COMMON__h                   

#ifdef __cplusplus

extern "C"{
#endif

////////////////////
// Hardware Setup //                                
////////////////////

///////////////////////////////////////////////////////////////////////////////
//  __________________________LED Strip____________________________________  //
//   
//  For WS2812 LED Strip: 
//   _______  __                          ____[WS2812 LED Strip connector]____
//  |__Arduino_ |                        |                                    |
//  |    +5V    |>---------------------->| V+  (usually Red cable)            |
//  |    GND    |>---------------------->| GND (usually White cable)          |
//  |  PIN_LED  |>---->[R 500 ohms]----->| DI  (data in - usually Green cable)|
//   \_________/                          \__________________________________/
//
//
//  For WS2813 LED Strip: 
//     WS2813 have a 4th cable: Backup data lines (BI- usually Blue cable)
//     Connect BI cable to GND 
//   ___________                          ____[WS2813 LED Strip connector]_____
//  |__Arduino_ |                        |                                     |
//  |    +5V    |>---------------------->| V+  (usually Red cable)             |
//  |    GND    |>----------------o----->| GND (usually White cable)           |
//  |           |                  \---->| BI  (backup in - usually Blue cable)|       
//  |  PIN_LED  |>---->[R 500 ohms]----->| DI  (data in - usually Green cable) |             
//   \_________/                          \___________________________________/
//  
//  __________________________Loudspeaker___________________________________  //
//
//   ___________                          _____________
//  |__Arduino_ |                        |             |
//  |    GND    |>--------------------->o| Loudspeaker |
//  | PIN_AUDIO |>----->[CAP 2uF]------>o|             |
//   \_________/                         |_____________|
//
///////////////////////////////////////////////////////////////////////////////
enum hw_setup {    // If you have a custom hardware (i.e not the OLR PCB), 
  PIN_LED = 2,     // set PIN_LED and PIN_AUDIO accordingly
  PIN_AUDIO =  3,   
};


///////////////////////////////////////////////////////////////////////////////
//  __________________ Digital Controllers (Buttons)_______________________  //
//
//   ________________               
//  |____Arduino____ |            ________________ 
//  |      GND       |>-------->o| Button 1 (Red) |
//  | DIG_CTRL_1_PIN |>-------->o|________________|
//  |                |            __________________
//  |      GND       |>-------->o| Button 2 (Green) |
//  | DIG_CTRL_2_PIN |>-------->o|__________________|
//  |                |            _________________
//  |      GND       |>-------->o| Button 3 (Blue) |
//  | DIG_CTRL_3_PIN |>-------->o|_________________|
//  |                |            __________________
//  |      GND       |>-------->o| Button 4 (White) |
//  | DIG_CTRL_4_PIN |>-------->o|__________________|
//   \______________/                        
//
///////////////////////////////////////////////////////////////////////////////
#define DIG_CTRL_1_PIN  A2 // switch player 1 to PIN and GND
#define DIG_CTRL_2_PIN  A0 // switch player 2 to PIN and GND
#define DIG_CTRL_3_PIN  A3 // switch player 3 to PIN and GND
#define DIG_CTRL_4_PIN  A1 // switch player 4 to PIN and GND


////////////////////
// Software Setup //                                
////////////////////

///////////////////////////////////////////////////////////////////////////////
// __________________Colors setup (Racing lights, Ramp, etc)________________ //
//
#define COLOR1         track.Color(255,0,0)     // Light controlled by DIG_CTRL_1_PIN
#define COLOR2         track.Color(0,255,0)     // Light controlled by DIG_CTRL_2_PIN
#define COLOR3         track.Color(0,0,255)     // Light controlled by DIG_CTRL_3_PIN
#define COLOR4         track.Color(255,255,255) // Light controlled by DIG_CTRL_4_PIN

#define COLOR_RAMP     track.Color(64,0,64)
#define COLOR_COIN     track.Color(40,34,0)
#define COLOR_BOXMARKS track.Color(64,64,0)
#define WARNING_BLINK_COLOR  track.Color(32,20,0)

#define LED_SEMAPHORE  12 // LED in the Stip used as a Semaphore (Countdown phase)

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// _______________________________Delays ___________________________________ //
//
enum delays_setup {    // If you have a custom hardware (i.e not the OLR PCB), 
  CONTDOWN_PHASE_DURATION      = 2000, // (mSec) 
  CONTDOWN_STARTSOUND_DURATION = 40,   // (mSec)
  NEWRACE_DELAY                = 5000, // (mSec)
  INACTIVITY_TIMEOUT_DELAY     = 300,  // (Sec)  When demo_mode is active, board goes into demo mode after this inactivity time 
  TELEMETRY_DELAY              = 250,  // (mSec) Telemetry data sent every TELEMETRY_DELAY mSec
};
///////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
// !!!!!!!!!!!!!!! DO NOT CHANGE ANYTHING BELOW !!!!!!!!!!!!!!!!! //
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
////////////////////////////////////////////////////////////////////



enum internal_setup {
  REC_COMMAND_BUFLEN = 32,// received command buffer size
                          // At the moment, the largest received command is RAMP CONFIGURATION (A)
                          //    ex: A1400,1430,1460,12,0[EOC] (for a 1500 LED strip)
                          // 21 CHAR  
                            
  TX_COMMAND_BUFLEN = 48, // send command buffer size
                          // At the moment, the largest send command is Q
                          //    ex: QTK:1500,1500,0,-1,60,0,0.006,0.015,1[EOC] (for a 1500 LED strip)
                          // 37 CHAR 
  
  MAX_PLAYERS = 4,        // DO NOT Change: Current software supports max 4 controllers 
};



#ifdef __cplusplus
} // extern "C"
#endif

#endif
