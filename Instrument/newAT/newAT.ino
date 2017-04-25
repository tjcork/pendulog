//0x6E400001B5A3F393E0A9E50E24DCCA9E





//#include <SoftwareSerial.h>

//SoftwareSerial BTSerial(14, 12); //RX|TX


void setup(){

  //pinMode(15, OUTPUT);
  //digitalWrite(15, LOW);
  //pinMode(0, OUTPUT);
  //digitalWrite(0, HIGH);

 
  
  Serial.begin(9600);
  
  Serial1.begin(9600); // default baud rate
  while(!Serial); //if it is an Arduino Micro
  //Serial.println("AT commands: ");

}

void loop(){

  //read from the HM-10 and print in the Serial
  if(Serial1.available())
    Serial.write(Serial1.read());
    
  //read from the Serial and print to the HM-10
  if(Serial.available())
    Serial1.write(Serial.read());

}

