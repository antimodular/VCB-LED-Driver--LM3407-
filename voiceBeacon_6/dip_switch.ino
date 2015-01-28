//dip switch
int dipAmt = 6;

int dipPins[] = {
 16,12,4,2,1,11}; //DIP Switch Pins\
byte transAddress;


void setup_dipSwitch(){

  for(int i = 0; i<dipAmt; i++){
    pinMode(dipPins[i], INPUT_PULLUP);	// sets the digital pin 2-5 as input
    //digitalWrite(dipPins[i], HIGH); //Set pullup resistor on
  }

  readDipSwitch();
}

void readDipSwitch(){
 // transAddress = address();
  myGroupID = address(); //transAddress;
startAddress = myGroupID * DMX_NUM_CHANNELS;

}

byte address(){
  int i,j=0;

  //Get the switches state
  for(i=0; i<dipAmt; i++){
    j = (j << 1) | !digitalRead(dipPins[i]);   // read the input pin. ! turns true in to false and vis versa
  }
  return j; //return address
}




