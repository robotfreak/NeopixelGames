# OLR-Arduino - Minimalist race game with LED strip
Software running on OLR Board (Arduino) implementing a 'Car Race' where a 'car' is a light moving along a LED Strip (WS2812/WS2813).  
Manage up to 4 'cars' and optionals Slope (with gravity effect), Pitlane, Electric cars mode (need to recharge battery in Pitlane), etc    
Players use push-button controllers to move the 'cars' - The quicker you push the button the faster the light (car) moves along the LED Strip.  
The software can receive configuration commands via Serial interface:  (--see: doc/OLR_Protocol_Serial.pdf)

## Standalone mode
The software running on OLR Board (Arduino) implements a race as described above where up to 4 people can play using the push buttons. 

## Network mode / Relay Race
A "NetworkClient" software, running on an external 'Host' (Computer, SBC,..) controls the Software running on OLR Board (Arduino).  
Together they implements a Relay Race where 'cars' starts a Race in Racetrack 'A', than goes to Racetrack 'B', etc.   
-- see: https://gitlab.com/open-led-race-network/networkclient

## Hardware
Tested on [Arduino Nano] and [Arduino Every].  

Change settings in [olr-settings.h] to adapt to your hardware configuration.

```
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
 
```
 


