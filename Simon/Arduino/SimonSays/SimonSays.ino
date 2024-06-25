/*Simon Says game. Now with sound effects. 
Originaly made by Robert Spann
Original code trimmed by digimike
This code varies alot from the original code
That is because Simon Says now has some new features:
  - LED's are replaced with a NeoPixel Ring with 16 RGB LED's
  - Sounds from the games Mario Bros., The Legend Of Zelda and Sonic The Hedgehog
  - Point system made with two sound effects (low Mario coin sound and the real Mario coin sound) and two light effects
  - Buttons are replaced with self-made capacative touchpads
All these features are coded by Mick Gerritsen
*/
#include "pitches.h"
#include "neopixelRing.h"
#include "fsr.h"
#include <EEPROM.h>

#define MAX_LEVEL 100

int speakerpin = 12;
int starttune[] = { NOTE_E5, NOTE_E5, NOTE_E5, NOTE_C5, NOTE_E5, NOTE_G5, NOTE_G4 };
int duration2[] = { 125, 250, 250, 125, 250, 500, 500 };
int note[] = { NOTE_A4, NOTE_AS4, NOTE_B4, NOTE_C5 };
int duration[] = { 200, 200, 200, 800 };
int gameovertune[] = { NOTE_C4, NOTE_C4, NOTE_B3, NOTE_AS3, NOTE_A3, NOTE_G3 };
int duration3[] = { 150, 150, 300, 300, 300, 1200 };

int sequence[MAX_LEVEL];
int sound[MAX_LEVEL];
int gamer_sequence[MAX_LEVEL];
int level = 1;
//int note = 0;
int velocity = 1000;

int tens = 0;
int ones = 0;
int hitens;
int hiones;
int highscoredelay = 30;
bool highScoreShow = false;
bool fail1 = false;
int addr1 = 0;
int addr2 = 1;
int addrspeed = 2;
bool restart = false;
bool startupTone = true;
//int pace = EEPROM.read(addrspeed);
int pace = 4;
void setup() {
  //hiones = EEPROM.read(0);
  //EEPROM.write(0,0);
  setupFsr();
  fsrRead3();
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined(__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'
  Serial.begin(9600);
  startSequence();
}

void startSequence() {
  pixelsOff();
  for (int thisNote = 0; thisNote < 7; thisNote++) {
    // play the next note:
    // Play the startup tune
    tone(speakerpin, starttune[thisNote]);
    // hold the note:
    if (thisNote == 3) {
      topLeft();
    }
    if (thisNote == 0 || thisNote == 1 || thisNote == 2) {
      botLeft();
    }
    if (thisNote == 4 || thisNote == 6) {
      botRight();
    }
    if (thisNote == 5) {
      topRight();
    }
    delay(duration2[thisNote]);
    // stop for the next note:
    noTone(speakerpin);
    //Startup melody to remain NeoPixel off while between notes
    pixelsOff();
    delay(25);
  }
  delay(200);
}

void loop() {
  //  start();
  if (level == 1) {
    generateSequence();
  }
  showSequence();
  checkSequence();
}

void test() {
  //these functions need give a boolean if a certain pad is pushed/touched
  fsrRead1();
  fsrRead2();
  fsrRead3();
  fsrRead4();
  if (touchInput1 == true) {  //Checking for button push
    topLeft();
    tone(speakerpin, NOTE_G3, 100);
    delay(300);
    pixelsOff();
    Serial.println(1);
  }

  if (touchInput2 == true) {
    botLeft();
    tone(speakerpin, NOTE_A3, 100);
    delay(300);
    pixelsOff();
    Serial.println(2);
  }

  if (touchInput3 == true) {
    botRight();
    tone(speakerpin, NOTE_B3, 100);
    delay(300);
    pixelsOff();
    Serial.println(3);
  }
  if (touchInput4 == true) {
    Serial.println("touched");
    topRight();
    tone(speakerpin, NOTE_C4, 100);
    delay(300);
    pixelsOff();
    Serial.println(4);
  }
}

void generateSequence() {
  int note;
  randomSeed(analogRead(6));  //more randomness

  for (int i = 0; i < MAX_LEVEL; i++) {
    sequence[i] = random(10, 14);

    switch (sequence[i]) {  //convert color to sound
      case 10:              //blue
        note = NOTE_G3;     //Fa
        break;
      case 11:           //yellow
        note = NOTE_A3;  //Mi
        break;
      case 12:           //red
        note = NOTE_B3;  //Re
        break;
      case 13:           //green
        note = NOTE_C4;  //Do
        break;
    }
    sound[i] = note;
  }
}

void showSequence() {

  for (int i = 0; i < level; i++) {
    switch (sequence[i]) {  //convert color to sound
      case 10:
        botLeft();  // blue
        break;
      case 11:
        botRight();  // yellow
        break;
      case 12:
        topRight();  // red
        break;
      case 13:
        topLeft();  // green
        break;
    }

    tone(speakerpin, sound[i]);
    delay(velocity);
    noTone(speakerpin);
    pixelsOff();
    delay(200);
  }
}

void checkSequence() {
  int flag = 0;  //flag correct sequence

  for (int i = 0; i < level; i++) {
    flag = 0;

    while (flag == 0) {

      fsrRead1();
      fsrRead2();
      fsrRead3();
      fsrRead4();
      if (touchInput1 == true) {  //Checking for button push
        topLeft();                // green
        tone(speakerpin, NOTE_C4, 100);
        delay(velocity);
        pixelsOff();
        Serial.println(1);
        gamer_sequence[i] = 13;
        flag = 1;
        delay(200);

        if (gamer_sequence[i] != sequence[i]) {
          wrongSequence();
          return;
        }
      }

      if (touchInput2 == true) {
        botLeft();  //blue
        tone(speakerpin, NOTE_G3, 100);
        delay(velocity);
        pixelsOff();
        Serial.println(2);
        gamer_sequence[i] = 10;
        flag = 1;
        delay(200);

        if (gamer_sequence[i] != sequence[i]) {
          wrongSequence();
          return;
        }
      }

      if (touchInput3 == true) {
        botRight();  //yellow
        tone(speakerpin, NOTE_A3, 100);
        delay(velocity);
        pixelsOff();
        Serial.println(3);
        gamer_sequence[i] = 11;
        flag = 1;
        delay(200);

        if (gamer_sequence[i] != sequence[i]) {
          wrongSequence();
          return;
        }
      }
      if (touchInput4 == true) {
        Serial.println("touched");
        topRight();  //red
        tone(speakerpin, NOTE_B3, 100);
        delay(velocity);
        pixelsOff();
        Serial.println(4);
        gamer_sequence[i] = 12;
        flag = 1;
        delay(200);

        if (gamer_sequence[i] != sequence[i]) {
          wrongSequence();
          return;
        }
      }
    }
  }
  delay(200);
  rightSequence();
}

void wrongSequence() {
  delay(500);
  fail();
  delay(2000);
  level = 1;
  velocity = 1000;
  startSequence();
}

void rightSequence() {
  delay(500);
  victory();
  delay(1000);
  pixelsOff();
  delay(1000);

  if (level < MAX_LEVEL) {
    level++;
  }
  velocity -= 50;  //increases difficulty
}


void fail() {  //Function used if the player fails to match the sequence
  delay(200);
  for (int thisNote = 0; thisNote < 6; thisNote++) {
    // play the next note:
    // Play the startup tune
    tone(speakerpin, gameovertune[thisNote]);
    // hold the note:
    if (thisNote == 1 || thisNote == 3 || thisNote == 5) {
      gameOver();
    }
    if (thisNote == 0 || thisNote == 2 || thisNote == 4) {
      pixelsOff();
    }
    delay(duration3[thisNote]);
    noTone(speakerpin);
  }
  delay(500);
  //start();
  pointCounter(blue, yellow);
}

void pointCounter(uint32_t c10, uint32_t c1) {
  //tens and ones are integers for how many points are scored
  uint32_t tens, ones;
  tens = level / 10;
  ones = level % 10;
  pixelsOff();
  delay(500);
  for (int j = 0; j < tens; j++) {
    for (int q = 0; q < 1; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 1) {
        strip.setPixelColor(i + q, c10);  //turn every pixel on
      }
      strip.show();
      tone(speakerpin, NOTE_B2, 100);
      delay(100);
      tone(speakerpin, NOTE_E3, 200);
      delay(400);
      pixelsOff();
      delay(400);
    }
  }
  for (int j = 0; j < ones; j++) {
    for (int q = 0; q < 1; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 1) {
        strip.setPixelColor(i + q, c1);  //turn every pixel on
      }
      strip.show();

      tone(speakerpin, NOTE_B5, 100);
      delay(100);
      tone(speakerpin, NOTE_E6, 200);
      delay(400);
      pixelsOff();
      delay(400);
    }
  }
  delay(100);
}
