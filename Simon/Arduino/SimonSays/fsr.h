const int FSR_PIN1 = A0; // Pin connected to FSR/resistor divider
const int FSR_PIN2 = A1; // Pin connected to FSR/resistor divider
const int FSR_PIN3 = A2; // Pin connected to FSR/resistor divider
const int FSR_PIN4 = A3; // Pin connected to FSR/resistor divider

bool touchInput1 = false;
bool touchInput2 = false;
bool touchInput3 = false;
bool touchInput4 = false;
int inputDelay = 0;
int treshold = 240;
void setupFsr() 
{
  Serial.begin(9600);
  pinMode(FSR_PIN1, INPUT);
  pinMode(FSR_PIN2, INPUT);
  pinMode(FSR_PIN3, INPUT);
  pinMode(FSR_PIN4, INPUT);
}

void fsrRead1() 
{
  int fsr1 = analogRead(FSR_PIN1);
  //Serial.println(fsr1);
  delay(inputDelay);
if (fsr1 > treshold){
        touchInput1 = true;
    }
   else if (fsr1 <= treshold){
      touchInput1 = false;
   }
    //Serial.println(touchInput1);
    //delay(10);
}
void fsrRead2() 
{
  int fsr2 = analogRead(FSR_PIN2);
  //Serial.println(fsr2);
  delay(inputDelay);
if (fsr2 > treshold){
        touchInput2 = true;
    }
   else if (fsr2 <= treshold){
      touchInput2 = false;
   }
    //Serial.println(touchInput2);
    //delay(10);
}
void fsrRead3() 
{
  int fsr3 = analogRead(FSR_PIN3);

  //Serial.println(fsr3);
  delay(inputDelay);
if (fsr3 > treshold){
        touchInput3 = true;
    }
   else if (fsr3 <= treshold){
      touchInput3 = false;
   }
    //Serial.println(touchInput3);
    //delay(10);
}
void fsrRead4() 
{
  int fsr4 = analogRead(FSR_PIN4);
  Serial.println(fsr4);
  delay(inputDelay);
if (fsr4 > treshold){
        touchInput4 = true;
    }
   else if (fsr4 <= treshold){
      touchInput4 = false;
   }
    //Serial.println(touchInput4);
    //delay(10);
}

