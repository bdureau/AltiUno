/*
  Model Rocket single deployment altimeter Ver 1.4
 Copyright Boris du Reau 2012-2021
 
 This is using a BMP085 presure sensor and an Attiny 85
 The following should fire the main at apogee if it is at least 50m above ground of the launch site
 
  
 For the BMP085 pressure sensor
 Connect VCC of the BMP085 sensor to 5.0V! make sure that you are using the 5V sensor (GY-65 model)
 Connect GND to Ground
 Connect SCL to i2c clock - pin 7 (PB2) of the ATtiny 85 
 Connect SDA to i2c data - pin 5 (PB0) of the ATtiny 85 
 EOC is not used, it signifies an end of conversion
 XCLR is a reset pin, also not used here
 
 The apogee is connected to pin 2 (PB3)
 The apogee continuity test is connected to pin 3 (PB4)
 The speaker/buzzer is connected to pin 6 (PB1)
 */

#include <TinyWireM.h>
#include <tinyBMP085.h>

#define DEBUG //=true

BMP085 bmp;

//ground level altitude
long initialAltitude;
//current altitude
long currAltitude;
//Apogee altitude
long apogeeAltitude;
long liftoffAltitude;
long lastAltitude;
//Our drogue has been ejected i.e: apogee has been detected
boolean apogeeHasFired =false;
//nbr of measures to do so that we are sure that apogee has been reached 
unsigned long measures =5;


// those have been changed for the ATtiny 85
const int pinApogee = 3;
const int pinApogeeContinuity = 4;
const int pinSpeaker = 1;

int nbrLongBeep=0;
int nbrShortBeep=0;

boolean NoBeep=false;

//Kalman Variables
float f_1=1.00000;  //cast as float
float kalman_x;
float kalman_x_last;
float kalman_p;
float kalman_p_last;
float kalman_k;
float kalman_q;
float kalman_r;
float kalman_x_temp;
float kalman_p_temp;
//float KAlt;
//end of Kalman Variables

void setup()
{
  //Initialise the output pin
  pinMode(pinApogee, OUTPUT);
  pinMode(pinSpeaker, OUTPUT);
  pinMode(pinApogeeContinuity, INPUT);
  //Make sure that the output are turned off
  digitalWrite(pinApogee, LOW);
  digitalWrite(pinSpeaker, LOW);

  //init Kalman filter
  KalmanInit();
  //Wire.begin();
  TinyWireM.begin();
  //Presure Sensor Initialisation
  //bmp.begin();
 // bmp.begin( BMP085_STANDARD);
     //Low res should work better at high speed
  bmp.begin( BMP085_ULTRALOWPOWER);
  //initialisation give the version of the altimeter
  //One long beep per major number and One short beep per minor revision
  //For example version 1.2 would be one long beep and 2 short beep
  beepAltiVersion(1,4);
  
  //our drogue has not been fired
  apogeeHasFired=false;
  // let's do some dummy altitude reading
  // to initialise the Kalman filter
  for (int i=0; i<50; i++){
    KalmanCalc(bmp.readAltitude());
   }
  //let's read the lauch site altitude
  long sum = 0;
  for (int i=0; i<10; i++){
      sum += KalmanCalc(bmp.readAltitude());
    delay(50); }
  initialAltitude = (sum / 10.0);
  
  lastAltitude = 0; 
  liftoffAltitude =  20;
  //number of measures to do to detect Apogee
  measures = 5;

}

void loop()
{
  //read current altitude
  currAltitude = (KalmanCalc(bmp.readAltitude())- initialAltitude);
  if (( currAltitude > liftoffAltitude) != true)
  {
    continuityCheck(pinApogeeContinuity);
  }
  
  //detect apogee
  if(currAltitude > liftoffAltitude)
  {

    if (currAltitude < lastAltitude)
    {
      measures = measures - 1;
      if (measures == 0)
      {
        //fire drogue
        digitalWrite(pinApogee, HIGH);
        delay (2000);
        apogeeHasFired=true;
        digitalWrite(pinApogee, LOW);
        //apogeeAltitude = currAltitude;
        apogeeAltitude = lastAltitude;
      }  
    }
    else 
    {
      lastAltitude = currAltitude;
      measures = 5;
    } 
  }

  if(apogeeHasFired == true)
  {
    beepAltitude(apogeeAltitude);
  }
}

void continuityCheck(int pin)
{
  int val = 0;     // variable to store the read value
  // read the input pin to check the continuity if apogee has not fired
  if (apogeeHasFired == false )
  {
    val = digitalRead(pin);   
    if (val == 0)
    {
      //no continuity long beep
      longBeep();
    }
    else
    {
      //continuity short beep
      shortBeep();
    }
  }
}



void beepAltitude(long altitude)
{
  int i;
  // this is the laste thing that I need to write, some code to beep the altitude
  //altitude is in meters
  //find how many digits
  if(altitude > 99)
  {
    // 1 long beep per hundred meter
    nbrLongBeep= int(altitude /100);
    //then calculate the number of short beep
    nbrShortBeep = (altitude - (nbrLongBeep * 100)) / 10;
  } 
  else
  {
    nbrLongBeep = 0;
    nbrShortBeep = (altitude/10); 
  }

  if (nbrLongBeep > 0)
  for (i = 1; i <  nbrLongBeep +1 ; i++)
  {
    longBeep();
    delay(50);
  } 

  if (nbrShortBeep > 0)
  for (i = 1; i <  nbrShortBeep +1 ; i++)
  {
    shortBeep();
    delay(50);
  } 

  delay(5000);

}

void beginBeepSeq()
{
  int i=0;
  if (NoBeep == false)
  {
    for (i=0; i<10;i++)
    {
      tone(pinSpeaker, 1600,1000);
      delay(50);
      noTone(pinSpeaker);
    }
    delay(1000);
  }
}
void longBeep()
{
  if (NoBeep == false)
  {
    tone(pinSpeaker, 600,1000);
    delay(1500);
    noTone(pinSpeaker);
  }
}
void shortBeep()
{
  if (NoBeep == false)
  {
    tone(pinSpeaker, 600,25);
    delay(300);
    noTone(pinSpeaker);
  }
}
//================================================================
// Kalman functions in your code
//================================================================

//Call KalmanInit() once.  

//KalmanInit() - Call before any iterations of KalmanCalc()
void KalmanInit()
{
   kalman_q=4.0001;  //filter parameters, you can play around with them
   kalman_r=.20001;  // but these values appear to be fairly optimal

   kalman_x = 0;
   kalman_p = 0;
   kalman_x_temp = 0;
   kalman_p_temp = 0;
   
   kalman_x_last = 0;
   kalman_p_last = 0;
   
}

//KalmanCalc() - Calculates new Kalman values from float value "altitude"
// This will be the ASL altitude during the flight, and the AGL altitude during dumps
float KalmanCalc (float altitude)
{
   
   //Predict kalman_x_temp, kalman_p_temp
   kalman_x_temp = kalman_x_last;
   kalman_p_temp = kalman_p_last + kalman_r;
   
   //Update kalman values
   kalman_k = (f_1/(kalman_p_temp + kalman_q)) * kalman_p_temp;
   kalman_x = kalman_x_temp + (kalman_k * (altitude - kalman_x_temp));
   kalman_p = (f_1 - kalman_k) * kalman_p_temp;
   
   //Save this state for next time
   kalman_x_last = kalman_x;
   kalman_p_last = kalman_p;
   
   //Assign current Kalman filtered altitude to working variables
   //KAlt = kalman_x; //FLOAT Kalman-filtered altitude value
  return kalman_x;
}  

void beepAltiVersion (int majorNbr, int minorNbr)
{
  int i;
  for (i=0; i<majorNbr;i++)
  {
    longBeep();
  }
  for (i=0; i<minorNbr;i++)
  {
    shortBeep();
  }  
}
