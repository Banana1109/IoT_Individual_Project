#include "DHT.h"
#define DHT_PIN 2
#define DHT_TYPE DHT11
#define TRIG_PIN 4
#define ECHO_PIN 8
//initialize moisutre sensor
DHT moistureSensor(DHT_PIN, DHT_TYPE);
const int switchPin = 7;
const int fanPin = 9;               
const int lightPin = 6;

// buffer time for when detect a person, avoid flickering
const unsigned long bufferTime = 100;              
// buffer time for when the user use the physic light switch, the state the light is in will stay the same until this time is up
const unsigned long bufferTime2 = 10 * 1000;             
// save the last time someone was in bathroom or touch the light switch
unsigned long lastPresenceTime = 0;                     
// last state the physical light switch was in, this is to detect the moment the user manually interact with the switch
int lastButtonState = LOW;                             
// this indicate that the user recently use the switch, stay in the same state for buffertime2 milisecond
// save last fan state to avoid bouncing
int lastFanState =LOW;
long lastFanTime=millis();
int persist=false;        
int persistFan=false;     
int messageTimer=0;        
float humid=0;                 
// track light on or off
int lightState=false;                                  
// Define a variable to store the duration of the echo pulse of presence sensor
long duration;  
// Define a variable to store the distance from the presence sensor in centimeters 
long distance;
//record the last time the program output the read of humid sensor
bool presence=false;
//the current threshold for the humid sensor to set the fan on
int threshold=75;
void setup() {
    Serial.begin(9600);
    pinMode(switchPin, INPUT);
    pinMode(fanPin, OUTPUT);
    pinMode(lightPin, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    moistureSensor.begin();
}

void loop() {
    
    //receive web input
    if (Serial.available()>0)
   {
      //Read serial input
      int value = Serial.parseInt();
      if (value == 1)
      {
        lightState=true;      
        persist=true;  
        lastPresenceTime = millis();
        digitalWrite(lightPin, HIGH);
        //Serial.println("LIGHT_ON Web");
      }
      else if (value == 2)
      {
        lightState=false;    
        persist=true;    
        lastPresenceTime = millis();
        digitalWrite(lightPin, LOW);
        //Serial.println("LIGHT_OFF Web");
      }
       else if (value == 3)
      {
        persistFan=true;
        digitalWrite(fanPin, HIGH);
        //Serial.println("FAN_ON Web");
        lastFanState=HIGH;
        lastFanTime=millis();
      }
       else if (value == 4)
      {
        persistFan=true;
        digitalWrite(fanPin, LOW);
        //Serial.println("FAN_OFF Web");
        lastFanState=LOW;
        lastFanTime=millis();
      }
      else {threshold=value;}
   }

    humid=moistureSensor.readHumidity();
    if (persistFan==true && millis() - lastFanTime > bufferTime2){     
      persistFan=false;
      //Serial.println("FAN Reset");
      //Serial.println(millis()-lastFanTime);
    }
    //if hight moisture then turn on fan
    if (humid > threshold && lastFanState==LOW && persistFan==false) {                   
        digitalWrite(fanPin, HIGH);
        Serial.println("FAN_ON Sensor");
        lastFanState=HIGH;
        lastFanTime=millis();
    } 

    
    //if no moisture then turn off fan
    if (humid <= threshold && lastFanState==HIGH && persistFan==false){                                                        
        digitalWrite(fanPin, LOW);
        Serial.println("FAN_OFF Sensor");
        lastFanState=LOW;
        lastFanTime=millis();
    }

    // check if the user manually interact with the ligth switch
    if(digitalRead(switchPin)==HIGH && lastButtonState==LOW) {                 
        // set the persist state to true so the presence sensor does not mess with the light for a while
        persist=true;      
        // set light state and output to the light                                                            
        lightState=!lightState;      
        lastPresenceTime = millis();       
                                                     
        if(lightState){
            digitalWrite(lightPin, HIGH);
            //Serial.println("LIGHT_ON Button");
        }
        else{
            digitalWrite(lightPin, LOW);
            //Serial.println("LIGHT_OFF Button");
        }
        lastButtonState=HIGH;
    }
    // Save the current button state to lastButtonState
    lastButtonState=digitalRead(switchPin);

    //trigger the ultrasonic sensor
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Read the duration of the echo pulse in microseconds
    duration = pulseIn(ECHO_PIN, HIGH);
    // get the distance by multiply with speed of sound, divide 2 because it's a round trip
    distance = duration * 0.034 / 2;
    if (distance<5){
      presence=true;
    }
    else{
      presence=false;
    }

    //update the last time the user was in the room while they are still detected in the room
    if (presence){                    
      lastPresenceTime = millis();
    }

    //check the lm, if it has been 5 minutes, reset the persist state to false
    if (persist && millis() - lastPresenceTime > bufferTime2){     
      persist=false;
    }
    

    //if somebody is in the room the the switch has not been pushed recently, turn on light
    if (presence && !persist && !lightState) {       
        lightState=true;
        digitalWrite(lightPin, HIGH);
        //Serial.println("LIGHT_ON Sensor");
    }

    //if none is in the room the the switch has not been pushed recently, turn off light
    if  (!presence && !persist  && millis() - lastPresenceTime > bufferTime  && lightState){
        lightState=false;
        digitalWrite(lightPin, LOW);
        //Serial.println("LIGHT_OFF Sensor");
    }

    if (millis()-messageTimer>500){
      String message=String(lastFanState)+","+String(lightState)+","+String(humid)+","+String(threshold);
      Serial.println(message);
      messageTimer=millis();
    }
}

