#include <Arduino.h>
#include <AltSoftSerial.h>

int resetPin = 12;
int blueLEDPin = 11;
int redLEDPin = 6; 
int greenLEDPin = 5; 
int buttonpin = 4;
int val; 

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)  
  AltSoftSerial BLEMini;  
#else
  #define BLEMini Serial1
#endif



#include <GSM.h>

#define PINNUMBER "2,3"

#define DIGITAL_OUT_PIN    7

GSM gsmAccess;
GSM_SMS sms;



unsigned long currentMillis;        // store the current value from millis()
unsigned long previousMillis;       // for comparison with currentMillis
int samplingInterval = 5;          // how often to run the main loop (in ms)

void setup()
{
  
  BLEMini.begin(57600);

  digitalWrite(resetPin, HIGH);
  delay(200);
  
  pinMode(DIGITAL_OUT_PIN, OUTPUT);

  pinMode(redLEDPin, OUTPUT);

  pinMode(greenLEDPin, OUTPUT);

  pinMode(buttonpin, INPUT);

  pinMode(resetPin, OUTPUT);

Serial.println("reset");
delay(200);

  static boolean analog_enabled = false;
  static byte old_state = LOW;

  

    // initialize serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println("BEAT THE HEAT CAR SEAT! STARTING UP!!");


  // connection state
  boolean notConnected = true;

  

  // Start GSM shield
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while (notConnected) {
    if (gsmAccess.begin(0000) == GSM_READY) {
      notConnected = false;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }


  Serial.println("READY, IF YOU FORGET CHILD YOU WILL RECIEVE TEXT");


}



int readSerial(char result[]) {
  int i = 0;
  while (1) {
    while (Serial.available() > 0) {
      char inChar = Serial.read();
      if (inChar == '\n') {
        result[i] = '\0';
        Serial.flush();
        return 0;
      }
      if (inChar != '\r') {
        result[i] = inChar;
        i++;
      }
    }
  }
}






void loop()
{
  
 
  
 while ( !BLEMini.available())
  {
    
    digitalWrite(blueLEDPin, LOW);
    digitalWrite(DIGITAL_OUT_PIN, HIGH);
    digitalWrite(redLEDPin, HIGH);
   
   
   val= digitalRead(buttonpin);


   if (val== LOW){

   digitalWrite(greenLEDPin, HIGH);
   
  //CODE FOR SIM900 HERE:

  Serial.print("NO PARENT DETECTED!!!!!! Texting Parent..... ");
  
  char remoteNumb[20] = "4048588657";  // telephone number to send sms
  //readSerial(remoteNumb);
  Serial.println(remoteNumb);

  // sms text
  Serial.print("SMS content:CHILD POSSIBLY LEFT IN CAR OR BLUETOOTH DISCONNECTED!!!");
  char txtMsg[200] = "CHILD LEFT IN CAR OR BLUETOOTH DISCONNECTED!";
  //readSerial(txtMsg);
  Serial.println("SENDING");
  Serial.println();
  Serial.println("Message:");
  Serial.println(txtMsg);

  // send the message
  sms.beginSMS(remoteNumb);
  sms.print(txtMsg);
  sms.endSMS();
  Serial.println("\nCOMPLETE!\n");
/////

delay(15000);
      digitalWrite(resetPin, LOW);
      Serial.println("RESTARTING");
   } 
 }       


 

 
 
 
 while ( BLEMini.available())
  {
    
    digitalWrite(blueLEDPin, HIGH);
    digitalWrite(DIGITAL_OUT_PIN, LOW);
    digitalWrite(redLEDPin, LOW);
  
  
       

  val= digitalRead(buttonpin); 
  
  if (val== HIGH)

    {

      digitalWrite(greenLEDPin, LOW);
      
      digitalWrite(redLEDPin, HIGH);

    }


 
  else if (val== LOW)

   {

      digitalWrite(redLEDPin, LOW);
    
      digitalWrite(greenLEDPin, HIGH);
      delay(15000);
      digitalWrite(resetPin, LOW);
      Serial.println("RESTARTING");
   }
  
  
  } 


delay(50);
}


