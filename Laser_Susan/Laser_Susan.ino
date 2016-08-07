#include <LiquidCrystal.h>
// DEFINE EXPERIMENTAL CONDITIONS ##############################################
// WELLS are numbered A B C D E F
// 30 Hz train lasting 1 s and repeated every 10 s for 10 min for either 24hrs || 12 hrs on / 12hrs off
// EXPERIMENT START DATE 9/8/15
const int wellCount = 6; // # of wells, useful
const float freq = 30; // frequency of stimulation in Hz
//const float freq = frequency; //catch for our laziness
long train[] = {1000L, 9000L}; // parameters of the train, in milliseconds {how long to stimulate, repetition interval}
long wellTime = 60000L * 10;  // how long to wait on each well, in milliseconds, * [__ minutes]
char wellLabels[] = "ABCDEF";
long wellDurations[] = {0,0,0,0,0,0};
float wellCounter = 0;
float revolutions = 0;
float totalRevolutions = 12;
// pulse length will be 1 ms in all conditions

long startTime;
long elapsedTime;

// SET PIN NUMBERS #############################################################
const int outPin = 13 ; // output voltage to AC power switch, which drives the motor
const int laserPin = 11; // output to laser (LED)
const int sensorPin = 2; // input from proximity sensor
const int buttonPin = A0; // input from LCD keypad
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // initializes LCD shield, arguments are pin numbers


void setup(){ // SETUP LOOP ####################################################
    int selected = 0;
    int buttonRead = 0;
    // initialize input and output pins
    pinMode(outPin, OUTPUT);
    pinMode(laserPin, OUTPUT);
    pinMode(sensorPin, INPUT);

    Serial.begin(9600);
    lcd.begin(16, 2); // initialize LCD display (sainsmart) of dimension 2 by 16
    lcd.clear(); // clear LCD
    lcd.setCursor(0, 0); // set cursor at initial segment
    lcd.print("LASER SUSAN");
    delay(1000);
    
    do{
      lcd.setCursor(0,1);
      lcd.print("SELECT > A");
      moveToWell();
      lcd.setCursor(0,1);
      lcd.print("THIS ONE?  ");
      delay(1000);
      buttonRead = analogRead(buttonPin);
      if(buttonRead < 800 && buttonRead > 700){
        selected = 1;
      }
    }while(selected == 0);
    startTime = millis();
}

void loop(){ // MAIN LOOP ######################################################
    for (int well = 0; well < wellCount; well++){
      // PRINT WELL NUMBER AND HZ
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(">");
      lcd.print(wellLabels[well]);
      lcd.print(" ");
      lcd.print(freq,0);
      lcd.print("Hz R:");
      lcd.print(revolutions,2);

      elapsedTime = millis() - startTime;
      Serial.print("Revolutions: ");
      Serial.println(revolutions);
      Serial.print("^Time Elapsed: ");
      Serial.print(h_fraction(elapsedTime));
      Serial.print("h");
      Serial.print(m_fraction(elapsedTime));
      Serial.print("m");
      Serial.print(s_fraction(elapsedTime));
      Serial.println("s");
      
      wellStimulate(freq, train[0], train[1], wellTime, well);
      wellCounter = wellCounter + 1; // add to wells covered;
      moveToWell();
      revolutions = wellCounter / 6.0; // keep track of # of revolutions
      //Serial.print(revolutions); //print revolutions in real time
      
    }
    if(revolutions == totalRevolutions){
      lcd.clear();
      lcd.print("EXP DONE");
      elapsedTime = millis() - startTime;
      Serial.print("Revolutions: ");
      Serial.println(revolutions);
      Serial.print("^Time Elapsed: ");
      Serial.print(h_fraction(elapsedTime));
      Serial.print("h");
      Serial.print(m_fraction(elapsedTime));
      Serial.print("m");
      Serial.print(s_fraction(elapsedTime));
      Serial.println("s");
      Serial.println("EXP DONE");
      exit(0);
    }

} 

// HELPER FUNCTIONS ############################################################
void wellStimulate(float frequency, long pulsetime, long pulseinterval, long duration, int wellIndex)
{ // stimulate as dictated by experimental parameters, passed in milliseconds
  long startTime;
  long currentTime;
  long pulseTimer;
  long wellDuration;
  startTime = millis();
  pulseTimer = startTime;
  currentTime = startTime;
  wellDuration = wellDurations[wellIndex]; // start off with initial duration
  laserStimulate(frequency, pulsetime); // stimulate to begin
  do {
    lcd.setCursor(0,1);
    lcd.print("t:");
    lcd.print(m_fraction(currentTime-startTime),0);
    lcd.print("m");
    lcd.print(s_fraction(currentTime-startTime),0);
    lcd.print("s ");
    lcd.setCursor(8,1);
    lcd.print("T:");
    
    currentTime = millis();
    if((currentTime - pulseTimer) >= pulseinterval + pulsetime){
      pulseTimer = currentTime;
      laserStimulate(frequency, pulsetime);
    }
    lcd.print(h_fraction(wellDuration + currentTime - startTime),0);
    lcd.print("h");
    lcd.print(m_fraction(wellDuration + currentTime - startTime),0);
    lcd.print("m");
    
  } while(currentTime - startTime < duration);
 wellDurations[wellIndex] = wellDuration + duration;
}

void laserStimulate(float f, long duration)
{ 
  long start = millis();
  long current = start;
  long timer = start;
  float cycle_time = 1000.0/f;
  do {
    current = millis();
    if(current - timer >= cycle_time){
      timer = current;
      digitalWrite(laserPin,HIGH);
      delay(1);
      digitalWrite(laserPin,LOW);
    }
  }while(current - start <= duration);
}

void moveToWell()
{ // this method turns the motor until the laser is centered over the next well
  int sensorValue;
  sensorValue = digitalRead(sensorPin); // read the sensor
  if(sensorValue == 0){ // clear the sensor of the current well, if present
    digitalWrite(outPin, HIGH);
    delay(5500);
  }
  do { // move until finds a new well
    sensorValue = digitalRead(sensorPin);
    digitalWrite(outPin, HIGH);
  } while (sensorValue == 1);

  digitalWrite(outPin, LOW);
  delay(200); // initial delay upon discovering well, reduce momentum
  digitalWrite(outPin, HIGH);
  delay(1200); // delay to center on well
  digitalWrite(outPin, LOW); // stop!!
}

float h_fraction(long x){ // return the hours fraction of an interval given in milliseconds
  return((float(x)-float(x%3600000))/3600000L);
}

float m_fraction(long y){ // return the minutes fraction of an interval given in milliseconds
  return(long((float(y)-float(y%60000))/60000L)%60); 
}

float s_fraction(long z){ // returns the seconds fraction of an interval given in milliseconds
  return(float(z%60000)/1000);
}


