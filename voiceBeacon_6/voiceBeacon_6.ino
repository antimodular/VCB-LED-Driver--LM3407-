//this works fine for teensy


#include <DmxReceiver.h>
DmxReceiver DMX = DmxReceiver();

int boardLED_Pin = 13;
boolean boardLED_state = true;

boolean bDebug = false; //true; //

//--------------------------------------dimming--------------------------------


// Create an IntervalTimer object 
//http://forum.pjrc.com/threads/24599-IntervalTimer-and-LED-fading?p=38067&viewfull=1#post38067
IntervalTimer dimmingTimer;

volatile unsigned int counter = 0;

const int counterLimit = 255; //160;

volatile  int dimDir [] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
volatile boolean dimStates [] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile boolean old_dimStates [] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile int dimValue [] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//Updated pin #'s to match final board
const char ledPin[] = {
  8,15,5,3,9,10,6,23,22,21,20, 19,18,17,14};

boolean bIsHardwarePWM[] = {
  0,0,1,1,1,1,1,1,1,1,1,0,0,0,0};


//---------------------------------------DMX-------------------------------------
#define DMX_NUM_CHANNELS 15

volatile unsigned char dmx_data[DMX_NUM_CHANNELS+3];
volatile unsigned char data;

//used for software PWMing
float newInterruptMS = 30; //54.0; //23.0;
float old_newInterruptMS = newInterruptMS;

//used for hardware PWMing via analogWrite
int newHardPWM = 500; //411;
int old_newHardPWM = newHardPWM;

//IntervalTimer dmxTimer;

//------------------auto dimming------------
unsigned long fadeTimer;
int maxVal = 255; //250 -> in australia
int minVal = 0;

int fadeDirs[DMX_NUM_CHANNELS];
int fadeValue[DMX_NUM_CHANNELS];
int fadeSteps[DMX_NUM_CHANNELS];


//--------------------------------------dip switch--------------------------------
byte myGroupID = 0; //default will be changed once DIP switch is read
  int startAddress;

//-------------------------------------init-------------------------------
boolean bInitDone =false; // true; // 
unsigned long startTimer;

int initStage = 0;

//void dmxTimerISR(void)
//{
//        DMX.bufferService();
//}

void setup(void) {

  if(bDebug){
    Serial.begin(9600);
    Serial.println("voiceBeacon_5");
  }

  setup_dipSwitch();

  //analogWriteResolution(8);
  for(int i=0; i<DMX_NUM_CHANNELS; i++){
    if(bIsHardwarePWM[i] == 1){
      //375000 = 375kHz
      analogWriteFrequency(ledPin[i],newHardPWM);
    }
    else{
      pinMode(ledPin[i],OUTPUT);
    }
  }

  dimmingTimer.begin(dimming_handler2, newInterruptMS);  


  randomSeed(analogRead(13));
  for(int i=0; i<DMX_NUM_CHANNELS; i++){
    fadeValue[i] = 0; //random(maxVal);
    fadeDirs[i] = 1;
    fadeSteps[i] = random(1,10);//20);
  }


  if(bInitDone == true) DMX.begin();

  pinMode(boardLED_Pin,OUTPUT);
  digitalWrite(boardLED_Pin,HIGH);

  DMX.begin();
    /* Use a timer to service DMX buffers every 1ms */
    //    dmxTimer.begin(dmxTimerISR, 1000);
}

void loop(){

  if(bInitDone == true) {
    dmxDimming();
    // autoDimming();
    // allOn();
  }
  else{


    if(initStage == 0 && millis() > 2000){
      initStage++;
      startTimer = millis();

      if(bDebug){
        Serial.print("DIP switch = ");
        Serial.print(myGroupID,BIN);
        Serial.print(", groupID = ");
        Serial.print(myGroupID,DEC);
       
        Serial.print(", startAddress ");
        Serial.print(startAddress);
        
         Serial.println();
        
      }
  
    }
    if(initStage == 1){
      for(int i=0; i<DMX_NUM_CHANNELS; i++){
        dmx_data[i] = 0;
      }

      int temp_i = map(millis(), startTimer, startTimer+1000, 0,DMX_NUM_CHANNELS); //(int)(millis()-startTimer) % 300;
      dmx_data[temp_i] = 60;

      if(temp_i > DMX_NUM_CHANNELS-1){
        dmx_data[myGroupID%DMX_NUM_CHANNELS] = 60;
        initStage++;
      }
    }

    if(initStage == 2 && millis() > 4000){
      //  DMX.begin();
      initStage++;
      bInitDone = true;
      digitalWrite(boardLED_Pin,LOW);
    }
  }



  for(int i=0; i<DMX_NUM_CHANNELS; i++){
    if(bIsHardwarePWM[i] == 1){
      // if(i == 0)Serial.println(dmx_data[i]);
      analogWrite(ledPin[i],dmx_data[i]);
    }
  }

}

void dmxDimming(){
  DMX.bufferService();

  if (DMX.newFrame()) {
  
    for(int i=0; i<DMX_NUM_CHANNELS; i++){

     // dmx_data[i] = dmx_getDimmer(startAddress + i + 1);
     dmx_data[i] = DMX.getDimmer(startAddress + i + 1);
     //Serial.print(dmx_data[i]);
    }

  }
}


void autoDimming(){
  if(millis() - fadeTimer > 20){
    fadeTimer = millis();

    for(int i=0; i<DMX_NUM_CHANNELS; i++){
      fadeValue[i] += (fadeDirs[i]*fadeSteps[i]);


      if(fadeValue[i] >= 255){
        //  if( i == 0) Serial.println(">");
        fadeDirs[i] = -1;
        fadeValue[i] = 255;
      }
      if(fadeValue[i] <= 0){
        //  if( i == 0) Serial.println("<");
        fadeDirs[i] = 1;
        fadeValue[i] = 0;
      }
      // if( i == 0) Serial.println(fadeValue[i]);
    }

  }

  for(int i=0; i<DMX_NUM_CHANNELS; i++){

    dmx_data[i] = fadeValue[i];

  }

}




void dimming_handler2(void) {

  for(int i=0; i<DMX_NUM_CHANNELS; i++){
    if(bIsHardwarePWM[i] == 0){

      if(dmx_data[i] > counter){
        dimStates[i] = true;
      }
      else{
        dimStates[i] = false;
      }

      if(dimStates[i] != old_dimStates[i]) {
        old_dimStates[i] = dimStates[i];
        digitalWriteFast(ledPin[i],dimStates[i]);
      }

    }
  }

 // digitalWrite(boardLED_Pin,dimStates[0]);
  counter++;
  if(counter > counterLimit) counter = 0;

}

void allOn(){
  for(int i=0; i<DMX_NUM_CHANNELS; i++){

    dmx_data[i] = 255;

  }
}












