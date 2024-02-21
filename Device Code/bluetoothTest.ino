include "Boards.h"
#include <ble_mini.h>

#define PROTOCOL_MAJOR_VERSION   0 //
#define PROTOCOL_MINOR_VERSION   0 //
#define PROTOCOL_BUGFIX_VERSION  2 // bugfix

#define PIN_CAPABILITY_NONE      0x00
#define PIN_CAPABILITY_DIGITAL   0x01
#define PIN_CAPABILITY_ANALOG    0x02


// pin modes
//#define INPUT                 0x00 // defined in wiring.h
//#define OUTPUT                0x01 // defined in wiring.h
#define ANALOG                  0x02 // analog pin in analogInput mode

byte pin_mode[TOTAL_PINS];
byte pin_state[TOTAL_PINS];




/* timer variables */
unsigned long currentMillis;        // store the current value from millis()
unsigned long previousMillis;       // for comparison with currentMillis
int samplingInterval = 250;          // how often to run the main loop (in ms)

void setup()
{
  BLEMini_begin(57600);
  
  #if !defined(__AVR_ATmega328P__)
  Serial.begin(57600);
  while(!Serial);  
  Serial.println("BLE Arduino Slave ");
  #endif

  /* Default all to digital OUTPUT */
  for (int pin = 0; pin < TOTAL_PINS; pin++)
  {
    // Set pin to input with internal pull up
    if(IS_PIN_DIGITAL(pin))
    {
      pinMode(pin, OUTPUT);
    }      
    digitalWrite(pin, LOW);
    // Save pin mode and state
    pin_mode[pin] = INPUT;
    pin_state[pin] = LOW;
  } 
}

static byte buf_len = 0;

byte reportDigitalInput()
{
  static byte pin = 0;
  byte report = 0;
  
  if (!IS_PIN_DIGITAL(pin))
  {
    pin++;
    if (pin >= TOTAL_PINS)
      pin = 0;
    return 0;
  }
  
  if (pin_mode[pin] == INPUT)
  {
      byte current_state = digitalRead(pin);
            
      if (pin_state[pin] != current_state)
      {
        pin_state[pin] = current_state;
        byte buf[] = {'G', pin, INPUT, current_state};
        BLEMini_write_bytes(buf, 4);
        
        report = 1;
      }
  }
  
  pin++;
  if (pin >= TOTAL_PINS)
    pin = 0;
    
  return report;
}

void reportPinCapability(byte pin)
{
  byte buf[] = {'P', pin, 0x00};
  byte pin_cap = 0;
                    
  if (IS_PIN_DIGITAL(pin))
    pin_cap |= PIN_CAPABILITY_DIGITAL;
            
  if (IS_PIN_ANALOG(pin))
    pin_cap |= PIN_CAPABILITY_ANALOG;

  buf[2] = pin_cap;
  BLEMini_write_bytes(buf, 3);
}



byte reportPinAnalogData()
{
  static byte pin = 0;
  byte report = 0;
  
  if (!IS_PIN_DIGITAL(pin))
  {
    pin++;
    if (pin >= TOTAL_PINS)
      pin = 0;
    return 0;
  }
  
  if (pin_mode[pin] == ANALOG)
  {
    uint16_t value = analogRead(pin);
    byte value_lo = value;
    byte value_hi = value>>8;
    
    byte mode = pin_mode[pin];
    mode = (value_hi << 4) | mode;
  
    byte buf[] = {'G', pin, mode, value};         
    BLEMini_write_bytes(buf, 4);
  }
  
  pin++;
  if (pin >= TOTAL_PINS)
    pin = 0;
    
  return report;
}

void reportPinDigitalData(byte pin)
{
  byte state = digitalRead(pin);
  byte mode = pin_mode[pin];
  byte buf[] = {'G', pin, mode, state};         
  BLEMini_write_bytes(buf, 4);
}



void sendCustomData(uint8_t *buf, uint8_t len)
{
  uint8_t data[20] = "Z";
  memcpy(&data[1], buf, len);
  BLEMini_write_bytes(data, len+1);
}

byte queryDone = false;

void loop()
{
  while(BLEMini_available())
  {
    byte cmd;
    cmd = BLEMini_read();

#if !defined(__AVR_ATmega328P__) // don't print out on UNO
    Serial.write(cmd);
#endif

    // Parse data here
    switch (cmd)
    {
      case 'V': // query protocol version
        {
          queryDone = false;
          
          byte buf[] = {'V', 0x00, 0x00, 0x01};
          BLEMini_write_bytes(buf, 4);          
        }
        break;
      
      case 'C': // query board total pin count
        {
          byte buf[2];
          buf[0] = 'C';
          buf[1] = TOTAL_PINS; 
          BLEMini_write_bytes(buf, 2);
        }        
        break;
      
      case 'M': // query pin mode
        {  
          byte pin = BLEMini_read();
          byte buf[] = {'M', pin, pin_mode[pin]}; // report pin mode
          BLEMini_write_bytes(buf, 3);
        }  
        break;
      
      case 'S': // set pin mode
        {
          byte pin = BLEMini_read();
          byte mode = BLEMini_read();
          
          
  
          /* ToDo: check the mode is in its capability or not */
          /* assume always ok */
          if (mode != pin_mode[pin])
          {              
            pinMode(pin, mode);
            pin_mode[pin] = mode;
          
            if (mode == OUTPUT)
            {
              digitalWrite(pin, LOW);
              pin_state[pin] = LOW;
            }
            else if (mode == INPUT)
            {
              digitalWrite(pin, HIGH);
              pin_state[pin] = HIGH;
            }
            else if (mode == ANALOG)
            {
              if (IS_PIN_ANALOG(pin)) {
                if (IS_PIN_DIGITAL(pin)) {
                  pinMode(PIN_TO_DIGITAL(pin), LOW);
                }
              }
            }
            
            
          }
            
  //        if (mode == ANALOG)
  //          reportPinAnalogData(pin);
          if ( (mode == INPUT) || (mode == OUTPUT) )
            reportPinDigitalData(pin);
        }
        break;

      case 'G': // query pin data
        {
          byte pin = BLEMini_read();
          reportPinDigitalData(pin);
        }
        break;
        
      case 'T': // set pin digital state
        {
          byte pin = BLEMini_read();
          byte state = BLEMini_read();
          
          digitalWrite(pin, state);
          reportPinDigitalData(pin);
        }
        break;
      
      
      case 'A': // query all pin status
        for (int pin = 0; pin < TOTAL_PINS; pin++)
        {
          reportPinCapability(pin);
          if ( (pin_mode[pin] == INPUT) || (pin_mode[pin] == OUTPUT) )
            reportPinDigitalData(pin);

        }
        
        queryDone = true; 
        
        {
          uint8_t str[] = "ABC";
          sendCustomData(str, 3);
        }
       
        break;
          
      case 'P': // query pin capability
        {
          byte pin = BLEMini_read();
          reportPinCapability(pin);
        }
        break;
        
      case 'Z':
        {
          byte len = BLEMini_read();
          byte buf[len];
          for (int i=0;i<len;i++)
            buf[i] = BLEMini_read();

#if !defined(__AVR_ATmega328P__)  
          Serial.println("->");
          Serial.print("Received: ");
          Serial.print(len);
          Serial.println(" byte(s)");
#endif          
        }
    }
    
    return; // only do this task in this loop
  }

  // No input data, no commands, process analog data
//  if (!ble_connected())
//    queryDone = false; // reset query state
    
  if (queryDone) // only report data after the query state
  { 
    byte input_data_pending = reportDigitalInput();  
    if (input_data_pending)  
      return; // only do this task in this loop

    currentMillis = millis();
    if (currentMillis - previousMillis > samplingInterval)
    {
      previousMillis += samplingInterval;
  
      reportPinAnalogData();
    }
  }  
}

