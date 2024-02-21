int redLEDPin = 6; 
int greenLEDPin = 5; 
int buttonpin = 4;
int val; 

void setup() {
  
  pinMode(redLEDPin, OUTPUT);

  pinMode(greenLEDPin, OUTPUT);

  pinMode(buttonpin, INPUT);
  

}

void loop() {

  val= digitalRead(buttonpin);

  if (val== LOW)

    {
      digitalWrite(greenLEDPin, HIGH);
      
      digitalWrite(redLEDPin, LOW);
    }

else if (val== HIGH)

{

  digitalWrite(redLEDPin, HIGH);
  
  digitalWrite(greenLEDPin, LOW);
}
}
