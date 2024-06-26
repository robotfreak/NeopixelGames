/*
 * ____                     _      ______ _____    _____
  / __ \                   | |    |  ____|  __ \  |  __ \
 | |  | |_ __   ___ _ __   | |    | |__  | |  | | | |__) |__ _  ___ ___
 | |  | | '_ \ / _ \ '_ \  | |    |  __| | |  | | |  _  // _` |/ __/ _ \
 | |__| | |_) |  __/ | | | | |____| |____| |__| | | | \ \ (_| | (_|  __/
  \____/| .__/ \___|_| |_| |______|______|_____/  |_|  \_\__,_|\___\___|
        | |
        |_|
 Open LED Race
 An minimalist cars race for LED strip

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 Current Version by:
    LucaBuka  (https://gitlab.com/lucabuka)
    Gerardo Barbarov (gbarbarov AT singulardevices DOT com)  
    Angel Maldonado (https://gitlab.com/angeljmc) 

  Basen on an original idea and 2 players code by: 
    Gerardo Barbarov  for Arduino day Seville 2019
    https://github.com/gbarbarov/led-race
    
 Public Repository for this code:
   https://gitlab.com/open-led-race/olr-arduino

*/

/** 
 *  ____________HARDWARE CONFIGURATION____________________
 *  __                                                  __
 *  __  Compile-time settings (PIN_LED, PIN_AUDIO, etc) __
 *  __  are defined in [olr-settings.h] file            __
 *  ______________________________________________________  
 */


char const softwareId[] = "A4P0";  // A4P -> A = Open LED Race, 4P0 = Game ID (4P = 4 Players, 0=Type 0)
char const version[] = "0.9.9";

#include "open-led-race.h" 

#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "olr-lib.h"
#include "olr-controller.h"
#include "olr-param.h"
#include "SoftTimer.h"
#include "SerialCommand.h"


/*------------------------------------------------------*/
int win_music[] = {
  2637, 2637, 0, 2637,
  0, 2093, 2637, 0,
  3136
};
/*------------------------------------------------------*/


static race_t       race;
static car_t        cars[ MAX_PLAYERS ];
static controller_t switchs[ MAX_PLAYERS ];
static track_t      tck;
char                tracksID[ NUM_TRACKS ][2] ={"U","M","B","I","O"};

static int     countdown_phase=1;
static bool    countdown_new_phase=true;

static int const eeadrInfo = 0; 

// non blocking delays
SoftTimer startRace_delay = SoftTimer();   // Autostart, Countdown
SoftTimer demoMode_delay  = SoftTimer();   // Activate Demo Mode on inactivity
SoftTimer telemetry_delay = SoftTimer(0);   // Send Telemetry data

static byte s_motor=0;
static int t_beep=0;
static int f_beep=0;


char cmd[REC_COMMAND_BUFLEN]; // Stores command received by ReadSerialComand()
SerialCommand serialCommand = SerialCommand(cmd, REC_COMMAND_BUFLEN, EOL, &Serial); // get complete command from serial
char txbuff[TX_COMMAND_BUFLEN];

Adafruit_NeoPixel track;

static uint32_t car_color[]={
  COLOR1,
  COLOR2,
  COLOR3,
  COLOR4
};



/*
 * 
 */
void setup() {

  Serial.begin(115200);
  randomSeed( analogRead(A6) + analogRead(A7) );
  controller_setup( );
  param_load( &tck.cfg );

  track = Adafruit_NeoPixel( tck.cfg.track.nled_total, PIN_LED, NEO_GRB + NEO_KHZ800 );
  
  // First 2 controllers always active (Red, Green)
  race.numcars = 2;
  
  // Calculate actual players number
  if( controller_isActive( DIGITAL_CTRL[CTRL_3] ) || param_option_is_active(&tck.cfg, PLAYER_3_OPTION) || param_option_is_active(&tck.cfg, PLAYER_4_OPTION)  ) {
    ++race.numcars;
  }
  if( controller_isActive( DIGITAL_CTRL[CTRL_4] ) || param_option_is_active(&tck.cfg, PLAYER_4_OPTION)) {
    ++race.numcars;
  }

  // Check if DEMO mode is configured
  race.demo_mode = param_option_is_active(&tck.cfg, DEMO_MODE_OPTION);
  enum ctr_type current_mode = (race.demo_mode == true) ? DEMO_MODE : DIGITAL_MODE;

  // !!! Eliminare var current_mode ...mettere if contratto direttamente in f() call
  
  // Initialize Controllers for very player
  set_controllers_mode(race.numcars, current_mode ) ;

  // Initialize car for every player
  init_cars(race.numcars);

  track.begin();
  strip_clear( &tck , false); 

  // Check Box before Physic/Sound to allow user to have Box and Physics with no sound
  if(controller_isActive( DIGITAL_CTRL[CTRL_2]) || param_option_is_active(&tck.cfg, BOX_MODE_OPTION) ) { //push switch 2 on reset for activate boxes (pit lane)
    box_init( &tck );
    track_configure( &tck, tck.cfg.track.nled_total - tck.cfg.track.box_len );
    draw_box_entrypoint( &tck );
  } else{
    track_configure( &tck, 0 );
  }

  if( controller_isActive( DIGITAL_CTRL[CTRL_1])  || param_option_is_active(&tck.cfg, SLOPE_MODE_OPTION) ) { // push switch 1 on reset for activate physics
    ramp_init( &tck );    
    draw_ramp( &tck );
    track.show();
    delay(2000);
    if ( controller_isActive( DIGITAL_CTRL[CTRL_1] ) ) { //retain push switch  on reset for activate FX sound
                                              s_motor=1;
                                              tone(PIN_AUDIO,100);}
  }

  race.network_race = false;  // always starts in standalone mode 
  race.demo_mode_on_received = false;
  race.demo_mode_off_received = false;
  
  race.cfg.startline  = tck.cfg.race.startline;   // always true for Standalone mode
  race.cfg.nlap       = tck.cfg.race.nlap;        // NUMLAP; 
  race.cfg.nrepeat    = tck.cfg.race.nrepeat;     // always 1 for Standalone mode 
  race.cfg.finishline = tck.cfg.race.finishline;  // always true for Standalone mode

  startRace_delay.start(0); // first race starts with no delay
  race.phase = READY;   // READY is the first status for Standalone mode

//  last_activity_millis = millis();


}

/*
 *  
 */
void loop() {

    // look for commands received on serial 
    ack_t ack = manageSerialCommand();
    if(ack.rp != NOTHING){
      sendResponse(&ack);
    }

    // Exit DEMO mode when a Player touch a controller 
    if( race.demo_mode_off_received || (race.demo_mode && players_actity(race.numcars))  ){
      exit_demo_mode();
    } 
    // If demo_mode option is set in board configuration 
    //    -> Enter demo mode after INACTIVITY_TIMEOUT_DELAY sec
    if( race.demo_mode_on_received || (param_option_is_active(&tck.cfg, DEMO_MODE_OPTION) && race.demo_mode==false && ready_for_demo_mode()) ) {
      activate_demo_mode();
    }


    // PLEASE NOTE:
    //  DO NOT call  "track.show()" in the loop() while in configuration mode !!!
    //  It would mess up with Serial communication (receives only 2 bytes - if the 
    //  string sent by the host is longer, it gets lost)
    //  In other phases (READY, RACING, etc) ONLY 2 bytes are guaranteed to be 
    //  succesfully received - So "Enter Configuration Mode" command is just one byte (@)

  switch(race.phase) {
    
    case CONFIG:
       {
        if( race.newcfg ) {  // Exit_Config command received
          race.newcfg = false;
          countdownReset();
          startRace_delay.start(0); 
          // for Standalone mode, gets into READY status
          // for Network races gets into CONFIGURATION OK status
          race.phase = ( race.network_race == false ) ? READY : CONFIG_OK;
          send_phase( race.phase );
        }
       }
      break;

    case CONFIG_OK:     // OLR Network only
      { 
      ; // In a Relay Race the configuration is sent (via 'C' command) by the 
        // Host ("Nerwork Client" program running on another Computer)
        // When the board reach the CONFIG_OK status...it does nothing but wait for
        // the next Command coming form the Host.
        // Same thing for the IDLE status (reached at the end of a relay race)
        // In other words, in Network mode (Relay races), some Status changes comes from the Host
      }
      break;

    case READY:      
      { 
        bool goOn=false;

        if( race.cfg.startline ) { // Standalone: Always true - Network mode: only racetrack where race starts
                             
          if(param_option_is_active(&tck.cfg,AUTOSTART_MODE_OPTION) || race.demo_mode ) { // Autostart parameters ON 
            if(startRace_delay.elapsed()) goOn=true;                                      // Automatically start Countdown after a defined Delay
                                                                                          // Note: In DemoMode always use AutoStart
          } else {                                                     // Autostart OFF: 
            int pstart=0;                                              // Wait for every controller be active (button pressed) to start Countdown
            strip_clear( &tck, true );        
            for( int i = 0; i < race.numcars; ++i) {
              if (controller_getStatus(cars[i].ct)==false) {
                    track.setPixelColor(i,cars[i].color); 
                    pstart++;
              }
            }      
            track.setPixelColor(LED_SEMAPHORE , ((millis()/5)%64)*0x010100 );   
            track.show();
            // if every controller activated -> Ready for Countdown 
            if (pstart==race.numcars) goOn=true;
          };
        }

        if(goOn || (!race.cfg.startline)) { // Standalone mode is Ready for Countdown __OR__ Network mode and Race does not starts here
          for( int i = 0; i < race.numcars; ++i) {
            car_resetPosition( &cars[i], true );  
            cars[i].repeats = 0;
            cars[i].st = CAR_WAITING; // Network race -> cleanup status of previous race 
          }
          tck.ledcoin  = COIN_RESET;
          race.phase = COUNTDOWN;
          if(race.network_race != true) send_phase( race.phase ); 

          srand((unsigned long) analogRead(A6) + analogRead(A7)); // used in demo_mode (see olr_controllers.h)
        }
      }
      break;


    case COUNTDOWN:
      {
        if( race.cfg.startline ) { // Standalone: Always true - Network mode: only racetrack where race starts
          // Countdown: semaphore and tones
          if(start_race_done()) { // Countdown done 
            for( int i = 0; i < race.numcars; ++i ) {
              cars[i].st = CAR_ENTER;
            }
            race.phase = RACING;
            send_phase( race.phase );
          }
        }
      }
      break;
      
    case RACING:
       {
        strip_clear( &tck, true );
          
        if( box_isactive( &tck ) ) {
          if( tck.ledcoin == COIN_RESET ) {
            tck.ledcoin = COIN_WAIT;
            tck.ledtime = millis() + random(2000,7000);
          }
          if( tck.ledcoin > 0 )
            draw_coin( &tck );
          else if( millis() > tck.ledtime )
            tck.ledcoin = random( 20, tck.cfg.track.nled_aux - 20 );

        } else {  
          if ( param_option_is_active(&tck.cfg, BATTERY_MODE_OPTION) ) { // Battery Mode ON
            if( tck.ledcoin == COIN_RESET ) {
              tck.ledcoin = COIN_WAIT;
              tck.ledtime = millis() + random(3000,8000);
            }
            if( tck.ledcoin > 0 )
               draw_coin( &tck );
            else if( millis() > tck.ledtime )
              tck.ledcoin = random( LED_SEMAPHORE+4, tck.cfg.track.nled_main - 60);  //valid zone from random charge (semaphore to 1 meter before to start-finish position 
          }  
        }

        for( int i = 0; i < race.numcars; ++i ) {
          run_racecycle( &cars[i], i );
          if( cars[i].st == CAR_FINISH ) {
            race.phase = COMPLETE;
            race.winner = (byte) i;
            send_phase( race.phase );
            break;
          }
        }
  
        track.show();
        if (s_motor==1) tone(PIN_AUDIO,f_beep+int(cars[0].speed*440*1)+int(cars[1].speed*440*2)+int(cars[2].speed*440*3)+int(cars[3].speed*440*4));
        if (t_beep>0) {t_beep--;} else {f_beep=0;};

  
        // Send Telemetry data
        if(telemetry_delay.elapsed()) {
          print_cars_positions( cars );
          telemetry_delay.start(TELEMETRY_DELAY);
        }
        // ---------------- 
       }
      break;
 
    case COMPLETE :    
      {
        strip_clear( &tck, false );
        track.show();
        draw_winner( &tck, cars[race.winner].color );
        sound_winner( &tck, race.winner );
        strip_clear( &tck, false );
        track.show();
        
        startRace_delay.start(NEWRACE_DELAY);

        // for Standalone mode, gets into READY status
        // for Network races gets into IDLE statue
        race.phase = ( race.network_race == false ) ? READY : IDLE;
      }  
      break;

    case IDLE:          // OLR Network only
      { 
      ; // -- see comment in CONFIG_OK status
      }
      break;
      
    default:
    {
      sprintf( txbuff, "SwErr-01");
      printdebug( txbuff, WARNING );
      break;
    }
      
  } // switch race.phase
  
}

/**
 * 
 */
void set_controllers_mode(uint8_t numctrl, uint8_t mode ) {
  for( uint8_t i = 0; i < numctrl; ++i) {
    controller_init( &switchs[i], mode, DIGITAL_CTRL[i] );
  }
}

/**
 * 
 */
void init_cars(uint8_t numcars ) {
  for( uint8_t i = 0; i < numcars; ++i) {
    car_init( &cars[i], &switchs[i], car_color[i] );
  }
}

bool players_actity(uint8_t numcars ) {
  for( uint8_t i = 0; i < numcars; ++i) {
    if(controller_isActive(DIGITAL_CTRL[i]))
      return(true);
  }
  return(false);  
}

/*
 * Check if Controllers (players) were incative for more than INACTIVITY_TIMEOUT_DELAY sec
 */
bool ready_for_demo_mode(void) {
  if(players_actity(race.numcars)){
    demoMode_delay.start(INACTIVITY_TIMEOUT_DELAY * 1000); // Reset timeout when somebody is using controllers
  }
  return (demoMode_delay.elapsed());
}
  


/**
 * 
 */
void activate_demo_mode(void){
  race.demo_mode = true; 
  race.demo_mode_on_received = false; // reset flag 
  set_controllers_mode(race.numcars, DEMO_MODE ) ;
  race.winner=0; // force a fake winner (used in Status=Complete by draw_winner())
  race.phase = COMPLETE;

  sprintf(txbuff, "%c%d%c", 'M', 1 , EOL );
  serialCommand.sendCommand(txbuff);
  
}

/**
 * 
 */
void exit_demo_mode(void){
  race.demo_mode = false;
  race.demo_mode_off_received = false; // reset flag 
  set_controllers_mode(race.numcars, DIGITAL_MODE ) ;
  race.winner=0; // force a fake winner (used in Status=Complete by draw_winner())
  race.phase = COMPLETE;
    
  sprintf(txbuff, "%c%d%c", 'M', 0 , EOL );
  serialCommand.sendCommand(txbuff);
}

void send_phase( int phase ) {
  sprintf(txbuff, "R%d%c",phase,EOL);
  serialCommand.sendCommand(txbuff);
}


void run_racecycle( car_t *car, int caridx ) {
    struct cfgtrack const* cfg = &tck.cfg.track;

    // if( car->st == CAR_COMING ) {  // OLR Network only
    //   // To be implemented
    // } 
    
    if( car->st == CAR_ENTER ) {
        // Standalone mode => On Race start the Speed get RESET (speed=0)
        // Network race    => Car speed set when receiving the Car_Enter Serial command
        (race.network_race == true) ?  car_resetPosition( car, false  ) : car_resetPosition( car, true  ); 

        if( car->repeats < race.cfg.nrepeat )
          car->st = CAR_RACING;
        else
          car->st = CAR_GO_OUT;
    }
   
    if( car->st == CAR_RACING ) {
        update_track( &tck, car );
        car_updateController( car );
        draw_car( &tck, car );

        if( car->nlap == race.cfg.nlap 
              && !car->leaving
              && car->dist > ( cfg->nled_main*car->nlap - race.circ.outtunnel ) ) {
            car->leaving = true;
            car->st = CAR_LEAVING;
        } 

        if( car->nlap > race.cfg.nlap ) {
            ++car->repeats;
            car->st = CAR_GO_OUT;
        }

        if( car->repeats >= race.cfg.nrepeat 
              && race.cfg.finishline ) {
            car->st = CAR_FINISH;  
        }
    }
   
    if( car->st == CAR_LEAVING ) {      // OLR Network only
        car->st = CAR_RACING;
        sprintf( txbuff, "r%d%c", caridx + 1, EOL );
        serialCommand.sendCommand(txbuff);
    }

    if( car->st == CAR_GO_OUT ) {       // OLR Network only
        car->st = CAR_WAITING;
        //map car number in 3 higher bits and car speed in 5 lower bits
        byte const speed = car->speed * 10;
        byte const data = (caridx + 1) << 5 | ( 0b00011111 & speed ); 
        sprintf( txbuff, "s%c%c", data, EOL );
        serialCommand.sendCommand(txbuff);;
        car_resetPosition( car, true );
        car->trackID = NOT_TRACK;
    }

    if ( car->st == CAR_FINISH ){
        car->trackID = NOT_TRACK;
        sprintf( txbuff, "w%d%c", caridx + 1, EOL );
        serialCommand.sendCommand(txbuff);

        car_resetPosition(car, true);
    }
}


int get_relative_position( car_t* car ) {
    enum{
      MIN_RPOS = 0,
      MAX_RPOS = 99,
    };
    struct cfgtrack const* cfg = &tck.cfg.track;
    int trackdist = 0;
    int pos = 0;

    switch ( car->trackID ){
      case TRACK_MAIN:
        trackdist = (int)car->dist % cfg->nled_main;
        pos = map(trackdist, 0, cfg->nled_main -1, MIN_RPOS, MAX_RPOS);
      break;
      case TRACK_AUX:
        trackdist = (int)car->dist_aux;
        pos = map(trackdist, 0, cfg->nled_aux -1, MIN_RPOS, MAX_RPOS);
      break;
    }
    return pos;
}


void print_cars_positions( car_t* cars ) {
    
    bool outallcar = true;
    for( int i = 0; i < race.numcars; ++i)
      outallcar &= cars[i].st == CAR_WAITING;
    
    if ( outallcar ) return;
    
    for( int i = 0; i < race.numcars; ++i ) {
      int const rpos = get_relative_position( &cars[i] );
      sprintf( txbuff, "p%d%s%d,%d,%d%c", i + 1, tracksID[cars[i].trackID], cars[i].nlap, rpos,(int)cars[i].battery, EOL );
      serialCommand.sendCommand(txbuff);
    }
}



/* 
 *  non-blocking  
 */
boolean start_race_done( ) { 
  if(countdown_new_phase){
    countdown_new_phase=false;
    startRace_delay.start(CONTDOWN_PHASE_DURATION);
    strip_clear( &tck , true);
    switch(countdown_phase) {
      case 1:
        tone(PIN_AUDIO,400);      
        track.setPixelColor(LED_SEMAPHORE, track.Color(255,0,0));   
        break;  
      case 2:
        tone(PIN_AUDIO,600);      
        track.setPixelColor(LED_SEMAPHORE, track.Color(0,0,0));  
        track.setPixelColor(LED_SEMAPHORE-1, track.Color(255,255,0));    
        break;
      case 3:
        tone(PIN_AUDIO,1200);      
        track.setPixelColor(LED_SEMAPHORE-1, track.Color(0,0,0)); 
        track.setPixelColor(LED_SEMAPHORE-2,  track.Color(0,255,0));   
        break;
      case 4:
        startRace_delay.start(CONTDOWN_STARTSOUND_DURATION);
        tone(PIN_AUDIO,880);      
        track.setPixelColor(LED_SEMAPHORE-2, track.Color(0,0,0)); 
        track.setPixelColor(0,  track.Color(255,255,255));   
        break;
      case 5:
        noTone(PIN_AUDIO); 
        countdownReset();  // reset for next countdown
        return(true);
    }
    track.show();    
  }
  if(startRace_delay.elapsed()) {
    noTone(PIN_AUDIO);    
    countdown_new_phase=true;
    countdown_phase++;
  }
  return(false);
}

/*
 * 
 */
void  countdownReset() {
  countdown_phase=1;
  countdown_new_phase=true;
}


void sound_winner( track_t* tck, byte winner ) {
  int const msize = sizeof(win_music) / sizeof(int);
  for (int note = 0; note < msize; note++) {
    tone(PIN_AUDIO, win_music[note],200);
    delay(230);
    noTone(PIN_AUDIO);
  }
}


void strip_clear( track_t* tck, bool show_settings ) {
    struct cfgtrack const* cfg = &tck->cfg.track;
    for( int i=0; i < cfg->nled_main; i++)
        track.setPixelColor( i, track.Color(0,0,0) );

    for( int i=0; i < cfg->nled_aux; i++)
        track.setPixelColor( cfg->nled_main+i, track.Color(0,0,0) );

    if(show_settings) {
      if( ramp_isactive( tck ))  
        draw_ramp( tck );
      if( box_isactive( tck ) )  
        draw_box_entrypoint( tck );    
    }
}


void draw_coin( track_t* tck ) {
    struct cfgtrack const* cfg = &tck->cfg.track;
    track.setPixelColor( 1 + cfg->nled_main + cfg->nled_aux - tck->ledcoin,COLOR_COIN );   
}

void draw_winner( track_t* tck, uint32_t color) {
  struct cfgtrack const* cfg = &tck->cfg.track;
  for(int i=16; i < cfg->nled_main; i=i+(8 * cfg->nled_main / 300 )){
      track.setPixelColor( i , color );
      track.setPixelColor( i-16 ,0 );
      track.show();
  }
}

void draw_car_tail( track_t* tck, car_t* car ) {
    struct cfgtrack const* cfg = &tck->cfg.track;
    
    switch ( car->trackID ){
      case TRACK_MAIN:
        for(int i=0; i<= car->nlap; ++i )
          track.setPixelColor( ((word)car->dist % cfg->nled_main) + i, car->color );
      break;
      case TRACK_AUX:
        for(int i=0; i<= car->nlap; ++i )     
          track.setPixelColor( (word)(cfg->nled_main + cfg->nled_aux - car->dist_aux) + i, car->color);         
      break;
    }
}

void draw_car( track_t* tck, car_t* car ) {
    struct cfgtrack const* cfg = &tck->cfg.track;
    struct cfgbattery const* battery = &tck->cfg.battery;
    
    switch ( car->trackID ){
      case TRACK_MAIN:
        for(int i=0; i<=1; ++i )
          track.setPixelColor( ((word)car->dist % cfg->nled_main) - i, car->color );
        if(param_option_is_active(&tck->cfg, BATTERY_MODE_OPTION)){ // Battery Mode ON
          if ( car->charging==1 ) {
            track.setPixelColor( ((word)car->dist % cfg->nled_main) - 2, 0x010100 * 50*(millis()/(201-2*(byte)car->battery)%2));
          } else  if (car->battery <= battery->min)
                    if ((millis()%100)>50) track.setPixelColor( ((word)car->dist % cfg->nled_main) - 2, WARNING_BLINK_COLOR );                           
        }  
      break;
      case TRACK_AUX:
        for(int i=0; i<=1; ++i )     
          track.setPixelColor( (word)(cfg->nled_main + cfg->nled_aux - car->dist_aux) + i, car->color); 
        if(param_option_is_active(&tck->cfg, BATTERY_MODE_OPTION)){ // Battery Mode ON
          
          if ( car->charging==1 )  {
            track.setPixelColor( (word)(cfg->nled_main + cfg->nled_aux - car->dist_aux) + 2, 0x010100 * 50*(millis()/(201-2*(byte)car->battery)%2));
          } else  if (car->battery <= battery->min)
                  if ((millis()%100)>50)  
                    track.setPixelColor( (word)(cfg->nled_main + cfg->nled_aux - car->dist_aux) + 2, WARNING_BLINK_COLOR); 
        }          
      break;
    }
}


/*
 *  Display on LED Strip current values for Slope and Pitlane
 *  
 */
void show_cfgpars_onstrip(){
  strip_clear( &tck, true ); 
  track.show();  
}


/*
 * 
 */
void draw_ramp( track_t* _tck ) {
    struct cfgramp const* r = &_tck->cfg.ramp;
    byte dist = 0;
    byte intensity = 0;
    for( int i = r->init; i <= r->center; ++i ) {
      dist = r->center - r->init;
      intensity = ( 32 * (i - r->init) ) / dist;
      track.setPixelColor( i, track.Color( intensity,0,intensity ) );
    }
    for( int i = r->center; i <= r->end; ++i ) {
      dist = r->end - r->center;
      intensity = ( 32 * ( r->end - i ) ) / dist;
      track.setPixelColor( i, track.Color( intensity,0,intensity ) );
    }
}


/*
 * 
 */
void draw_box_entrypoint( track_t* _tck ) {
    struct cfgtrack const* cfg = &_tck->cfg.track;
    int out = cfg->nled_total - cfg->box_len; // Pit lane exit (race start)
    int in = out - cfg->box_len;              // Pit lane Entrance
    track.setPixelColor(in ,COLOR_BOXMARKS ); 
    track.setPixelColor(out  ,COLOR_BOXMARKS ); 
}



/* 
 *  Check Serial to see if there is a command ready to be processed
 *  
 */
ack_t manageSerialCommand() {

  ack_t ack = { .rp = NOTHING, .type = '\0' };
  
  int clen = serialCommand.checkSerial();
  if(clen == 0) return ack;       // No commands received

  if(clen  < 0) {                 // Error receiving command  
    sprintf( txbuff, "Error reading serial command:[%d]",clen);
    printdebug( txbuff, WARNING );
  }
  
  // clen > 0 ---> Command with length=clen ready in  cmd[]
  ack.rp=NOK;


// Debug only
//if(race.network_race == true) {
//  sprintf( txbuff, "Recv cmd:%s", cmd);
//  printdebug( txbuff, WARNING );
//}


  switch (cmd[0]) {
    
    case '#':                         // Handshake
      {
        ack.type = cmd[0];
        sprintf( txbuff, "#%c", EOL );
        serialCommand.sendCommand(txbuff);    
        ack.rp = NOTHING;
      }
      break;

  case  '@' :                         // Enter "Configuration Mode" status
    {
      ack.type = cmd[0];    
      if(race.phase != CONFIG) {// Ignore command if Board already in "Configure Mode"
         race.phase = CONFIG;
        enter_configuration_mode();
      }
      ack.rp = OK;
    }
    break;
  
  case '*' :                          // Exit "Configure Mode"
    {
      ack.type = cmd[0];
      if(race.phase == CONFIG) { // Ignore command if Board is not in "Configure Mode"
        race.newcfg = true;
      } 
      ack.rp = OK;
    }
    break;    
    
  case 'R' :                          // Set Race Phase
    {
      ack.type = cmd[0];
      uint8_t const phase = atoi( cmd + 1);
      /**
      *  Reintrodotto momentaneamente R1=enter config per testare RelayRace con Networkclient esistente
      // Does not accept anymore  R=1 as Enter Configuration / Use command @ instead
      if( 0 > phase || RACE_PHASES <= phase || phase == CONFIG) return ack; 
      race.phase = (enum phases) phase;
      ack.rp = OK;
      **/
      // Codice vecchio con R1=Enter Configuration
      if( 0 > phase || RACE_PHASES <= phase) return ack;
      race.phase = (enum phases) phase;
      ack.rp = OK;
      if ( race.phase == CONFIG ) { // accept R1 as a EnterConfigurationMode command - DEPRECATED
        enter_configuration_mode();
      }
    
    }
    break;

  case 'u' :  // Car Enter the Circuit - // OLR Network only
    {
      ack.type = cmd[0];
      byte const data = cmd[1];
      byte const ncar = 0b00000111 & ( data >> 5 );
      byte const speed = 0b00011111 & data;
      if( 0 >= ncar || race.numcars < ncar) return ack;
      cars[ncar-1].st = CAR_ENTER;
      cars[ncar-1].speed = (float) speed / 10;
      ack.rp = OK;
      ack.rp = NOTHING;
      if( false ) {
        sprintf( txbuff, "%s %d, %s %d, %s %d", "CAR: ", ncar, "STATUS: ", cars[ncar-1].st, "SPEED: ", (int)(cars[ncar-1].speed * 10) );
        printdebug( txbuff, LOG );
      }      
    }
    break;    
       
  case 't' :  // Car Coming into the Circuit - // OLR Network only
    {
      ack.type = cmd[0];
      byte const ncar = atoi( cmd + 1);
      if( 0 >= ncar || race.numcars < ncar) return ack;
      cars[ncar-1].st = CAR_COMING;
      ack.rp = OK;
      if ( false ) { 
        sprintf( txbuff, "%s %d, %s %d", "CAR: ", ncar, "STATUS: ", cars[ncar-1].st);
        printdebug( txbuff, LOG );
      }
    }
    break;    

  case 'w' :  // Car Wins the race - OLR Network only 
              // Standalone mode: Board _NEVER_ receives a 'w' command !!!
    {         // Network mode: 1) Command "w" sent by the Board where the race ends
              //               2) Every other participant (Board) receives 'w' command
      ack.type = cmd[0];
      byte const ncar = atoi( cmd + 1);
      if( 0 >= ncar || race.numcars < ncar) return ack;
      if( race.network_race && (!race.cfg.finishline)) { 
        race.winner = (byte) ncar-1;  // Set the Winner
      }
      ack.rp = NOTHING; 
    }
    break;    

  
  case 'C' :                          //Parse race configuration -> C1,2,3,0
    {
      ack.type = cmd[0];
  
      char * pch = strtok (cmd,"C");
      if( !pch ) return ack;
  
      pch = strtok (pch, "," );
      if( !pch ) return ack;
      int startline = atoi( pch );
  
      pch = strtok (NULL, ",");
      if( !pch ) return ack;
      int nlap = atoi( pch );
  
      pch = strtok (NULL, ",");
      if( !pch ) return ack;
      int nrepeat = atoi( pch );
  
      pch = strtok (NULL, ",");
      if( !pch ) return ack;
      int finishline = atoi( pch );
  
      int err = race_configure( &tck, startline, nlap, nrepeat, finishline);
      if( err ) return ack;
  
      race.cfg.startline  = tck.cfg.race.startline;
      race.cfg.nlap       = tck.cfg.race.nlap;
      race.cfg.nrepeat    = tck.cfg.race.nrepeat;
      race.cfg.finishline = tck.cfg.race.finishline;
      
      ack.rp = OK;
    }
    break;
    

  case 'T' :                          //Parse Track configuration -> Track length
    {
      ack.type = cmd[0];

      char * pch = strtok (cmd,"T");
      if( !pch ) return ack;
  
      int nled = atoi( cmd + 1 );
      int err = tracklen_configure( &tck, nled);
      if( err ) return ack;
      track_configure( &tck, 0);
      if( err ) return ack;
      
      ack.rp = OK;
      }
    break;  

  case 'B' :                          //Parse BoxLenght Configuration -> Blen,perm
    {
      ack.type = cmd[0];
  
      char * pch = strtok (cmd,"B");
      if( !pch ) return ack;
  
      pch = strtok (pch, "," );
      if( !pch ) return ack;
      int boxlen = atoi( pch );
  
      pch = strtok (NULL, "," );
      if( !pch ) return ack;
      int boxperm = atoi( pch );
  
      int err = boxlen_configure( &tck, boxlen, boxperm );
      if( err ) return ack;
     
      ack.rp = OK;
  
      // Force Pitlane ON, so "show_cfgpars_onstrip()" 
      // will show the new values, even if AlwaysON=false
      box_init(&tck); 
      show_cfgpars_onstrip();
      }
    break;

  case 'A' :                          // Parse Ramp configuration -> Astart,center,end,high,perm
    {
      ack.type = cmd[0];
  
      char * pch = strtok (cmd,"A");
      if( !pch ) return ack;
  
      pch = strtok (pch, "," );
      if( !pch ) return ack;
      int init = atoi( pch );
  
      pch = strtok (NULL, "," );
      if( !pch ) return ack;
      int center = atoi( pch );
  
      pch = strtok (NULL, "," );
      if( !pch ) return ack;
      int end = atoi( pch );
  
      pch = strtok (NULL, ",");
      if( !pch ) return ack;
      int high = atoi( pch );
  
      pch = strtok (NULL, ",");
      if( !pch ) return ack;
      int slopeperm = atoi( pch );
      
      uint8_t err = ramp_configure( &tck, init, center, end, high, slopeperm );
      if( err ) return ack;
      ack.rp = OK;
  
      // Force Ramp ON, so "show_cfgpars_onstrip()" 
      // will show the new values, even if AlwaysON=false
      ramp_init(&tck); 
      
      show_cfgpars_onstrip();
      }
    break;

   case 'E' :                          // Parse Battery configuration -> Edelta,min,boost,active
    {
      ack.type = cmd[0];
  
      char * pch = strtok (cmd,"E");
      if( !pch ) return ack;
  
      pch = strtok (pch, "," );
      if( !pch ) return ack;
      uint8_t delta = atoi( pch );
  
      pch = strtok (NULL, "," );
      if( !pch ) return ack;
      uint8_t min = atoi( pch );
  
      pch = strtok (NULL, "," );
      if( !pch ) return ack;
      uint8_t boost = atoi( pch );
  
      pch = strtok (NULL, ",");
      if( !pch ) return ack;
      uint8_t active = atoi( pch );
      
      uint8_t err = battery_configure( &tck, delta, min, boost, active );
      if( err ) return ack;
      ack.rp = OK;
  
      }
    break;

  case 'G' :                          // Parse Autostart configuration -> Gautostart
    {
      ack.type = cmd[0];
  
      char * pch = strtok (cmd,"G");
      if( !pch ) return ack;
  
      uint8_t autostart = atoi( cmd + 1 );
      uint8_t err = autostart_configure( &tck, autostart);
      if( err ) return ack;
      
      ack.rp = OK;
    }
    break;  

  case 'M' :              // Parse DEMO mode configuration
    {
      ack.type = cmd[0];
  
      char * pch = strtok (cmd,"M");
      if( !pch ) return ack;
  
      uint8_t demo = atoi( cmd + 1 );

      uint8_t err = demo_configure( &tck, demo);
      if( err ) return ack;
      ack.rp = OK;
      
      if(demo == 0) {
        race.demo_mode_off_received = true;
      } else if( race.demo_mode){
        race.demo_mode_on_received = true;
      }
      
    }
    break;  
    
  case 'P' :                          // Parse Player 3/4 configuration -> P[2|3|4]
    {
      ack.type = cmd[0];
  
      char * pch = strtok (cmd,"P");
      if( !pch ) return ack;
  
      uint8_t players_n = atoi( cmd + 1 );
      uint8_t err = players_n_configure( &tck, players_n);
      if( err ) return ack;
      
      ack.rp = OK;
      }
    break;  



       
  case 'K':                           // Parse Physic simulation parameters
    {
      ack.type = cmd[0];
  
      char * pch = strtok (cmd,"K");
      if( !pch ) return ack;
  
      pch = strtok (pch, "," );
      if( !pch ) return ack;
      float kgp = atof( pch );
  
      pch = strtok (NULL, "," );
      if( !pch ) return ack;
      float kfp = atof( pch );
  
      int err = physic_configure( &tck, kgp, kfp );
      if( err ) return ack;
      
      ack.rp = OK;
    }
    break;

  case 'H' :  // Tunnel configuration - // OLR Network only
    {
      ack.type = cmd[0];
      uint8_t const dtunnel = atoi( cmd + 1);
      if( 0 >= dtunnel || 254 < dtunnel) return ack;
      race.circ.outtunnel = dtunnel;
      ack.rp = OK;
      if ( false ) { //VERBOSE
        sprintf( txbuff, "%s %d", "TUNNEL: ", race.circ.outtunnel );
        printdebug( txbuff, LOG );
      }      
    }
    break;    

    
  case 'D' :                          // Load Default Parameters and store them in from EEPROM
    {
      ack.type = cmd[0]; 
      param_setdefault( &tck.cfg );
      EEPROM.put( eeadrInfo, tck.cfg ); // Save immediately 
      
      ack.rp = OK;
      
      // Update box/slope active in current Track Struct with values 
      // just loaded (for show_cfgpars_onstrip())
      struct cfgparam const* cfg = &tck.cfg;
      tck.boxactive  = param_option_is_active(&tck.cfg, BOX_MODE_OPTION);
      tck.rampactive  = param_option_is_active(&tck.cfg, SLOPE_MODE_OPTION);
  
      show_cfgpars_onstrip();
    }
    break;
         
  case ':' :                          // Set board Unique Id
    {
      struct brdinfo* info = &tck.cfg.info;
      ack.type = cmd[0];
      if( strlen(cmd + 1) > LEN_UID ) return ack;
      strcpy( info->uid, cmd + 1 );
      EEPROM.put( eeadrInfo, tck.cfg ); // Save immediately 
      ack.rp = OK;
    }
    break;

  case '$':                           // Get Board UID
    {
      sprintf( txbuff, "%s%s%c", "$", tck.cfg.info.uid, EOL );
      serialCommand.sendCommand(txbuff);
      ack.rp = NOTHING;
    }
    break;

  case '?' :                          // Get Software Id
    {
      sprintf( txbuff, "%s%s%c", "?", softwareId, EOL );
      serialCommand.sendCommand(txbuff);
      ack.rp = NOTHING;
    }
    break;
  
  case '%' :                          // Get Software Version
    {
      sprintf( txbuff, "%s%s%c", "%", version, EOL );
      serialCommand.sendCommand(txbuff);    
      ack.rp = NOTHING;
    }
    break;

  case 'n' :                          // Set "Network Race" mode (Relay race)
    {
      ack.type = cmd[0]; 
      race.network_race = true; 
      race.phase = COMPLETE;  // Immediatly ends the current race (if any)
      race.winner=0;          // Set a fake winner (used in Status=Complete by draw_winner())
      ack.rp = OK;
    }
    break;

  case  'Q':                          // Get current configuration Info
    {
      struct cfgparam const* cfg = &tck.cfg;
      sprintf( txbuff, "%s:%d,%d,%d,%d,%d,%d,%d.%03d,%d.%03d,%d%c", "QTK",
                                      cfg->track.nled_total,
                                      cfg->track.nled_main,
                                      cfg->track.nled_aux,
                                      cfg->track.init_aux,
                                      cfg->track.box_len,
                                      //cfg->track.box_alwaysOn,
                                      param_option_is_active(&tck.cfg, BOX_MODE_OPTION),
                                      (int)cfg->track.kg, (int)(cfg->track.kg*1000)%1000, // std arduino sprintf() missing %f
                                      (int)cfg->track.kf, (int)(cfg->track.kf*1000)%1000, // std arduino sprintf() missing %f
                                      param_option_is_active(&tck.cfg, AUTOSTART_MODE_OPTION),                                      
                                      EOL );
      serialCommand.sendCommand(txbuff);
  
      sprintf( txbuff, "%s:%d,%d,%d,%d,%d%c", "QRP",
                                      cfg->ramp.init,
                                      cfg->ramp.center,
                                      cfg->ramp.end,
                                      cfg->ramp.high,
                                      //cfg->ramp.alwaysOn,
                                      param_option_is_active(&tck.cfg, SLOPE_MODE_OPTION),
                                      EOL );
      serialCommand.sendCommand(txbuff);
  
      sprintf( txbuff, "%s:%d,%d,%d,%d%c", "QBT",
                                      cfg->battery.delta,
                                      cfg->battery.min,
                                      cfg->battery.speed_boost_scaler,
                                      param_option_is_active(&tck.cfg, BATTERY_MODE_OPTION),
                                      EOL );
      serialCommand.sendCommand(txbuff);
      
      sprintf( txbuff, "%s:%d,%d,%d,%d,%d,%d,%d,%d%c", "QRC",
                                      cfg->race.startline,
                                      cfg->race.nlap,
                                      cfg->race.nrepeat,
                                      cfg->race.finishline,
                                      param_option_is_active(&tck.cfg, PLAYER_3_OPTION),
                                      param_option_is_active(&tck.cfg, PLAYER_4_OPTION),
                                      param_option_is_active(&tck.cfg, DEMO_MODE_OPTION),
                                      //race.demo_mode,
                                      race.network_race,
                                      EOL );
      serialCommand.sendCommand(txbuff);
  
      ack.rp = NOTHING;
    }
    break;

  case 'W':                           // Write configuration to EEPROM
    {
      ack.type = cmd[0];
      EEPROM.put( eeadrInfo, tck.cfg );
      ack.rp = OK;
    }
    break;
    
  } // switch
  
  return(ack);
 
}

/*
 * 
 */
void sendResponse( ack_t *ack) {
  if(ack->type=='\0'){
    sprintf(txbuff, "%s%c", ack->rp==OK? "OK":"NOK" , EOL );
  } else {
    sprintf(txbuff, "%c%s%c", ack->type, ack->rp==OK? "OK":"NOK" , EOL );
  }
  serialCommand.sendCommand(txbuff);
}

/*
 *  Send Log/Warning/Error messages to host 
 */
void printdebug( const char * msg, int errlevel ) {
  char header [5];
  sprintf(header, "!%d,",errlevel);
  Serial.print(header);
  Serial.print(msg);
  Serial.print(EOL);
}


/*
 *  reset race parameters
 *  stop sound
 */
void enter_configuration_mode(){
    noTone(PIN_AUDIO); 
    strip_clear( &tck, false );
    track.show(); 
}


 
void param_load( struct cfgparam* cfg ) {

    /**    
    // Ignore EEPROM params during development of a new version of the [cfgparam]
    param_setdefault( &tck.cfg );
    sprintf( txbuff, "%s%c", "Temporary....DEFAULT PAREMETRS LOADED ", EOL );
    serialCommand.sendCommand(txbuff);
    return;
    **/

    EEPROM.get( eeadrInfo, tck.cfg );

//    sprintf( txbuff, "%s:%d%c", "EEPROM-v", tck.cfg.ver, EOL );
//    serialCommand.sendCommand(txbuff);

    if ( tck.cfg.ver != CFGPARAM_VER ) { // [cfgparam.ver] read form EEPROM != [#define CFGPARAM_VER] in the code
      // Each time a new version of the code modify the [cfgparam] struct, [#define CFGPARAM_VER] is also 
      // changed to force the code enter here. 
      // The previous values stored in EEPROM are invalid and need to be reset-to-default and
      // stored in the EEPROM again with the new "structure" 
      param_setdefault( &tck.cfg );
      EEPROM.put( eeadrInfo, tck.cfg );
      sprintf( txbuff, "%s:%d%c", "DEFAULT->EEPROM-v)", tck.cfg.ver, EOL );
      serialCommand.sendCommand(txbuff);
    }

}
