

#define BUFFER_LENGTH 100
char blebuf[BUFFER_LENGTH];
char* BLEstrings[4];



boolean parseBLE(char input[], char* strings[]) {
  char *ptr = NULL;
  byte index = 0;
  ptr = strtok(input, ";");  // takes a list of delimiters
  while(ptr != NULL and index < 4)
  {
      strings[index] = ptr;
      index++;
      ptr = strtok(NULL, ";");  // takes a list of delimiters
  }
  if (index == 4)
    return true;
  else
    Serial.print("INDEX =  "); Serial.println(index);
  return false;
}





  



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  

}

int i=0;

void loop() {
  // put your main code here, to run repeatedly:
  

  if(Serial.available()) {
    delay(100);
     while(Serial.available()) {    // loop and read the data
        char a=Serial.read();
        Serial.print("\"");Serial.print(a);Serial.print("\", ");
        if (a=='\r'){
          Serial.read();
          break;
        }
        blebuf[i]=a;          // save data to buffer
        i++;
        if (i>=BUFFER_LENGTH) break;  // prevent buffer overflow, need to break
        delay(1);           // give it a 2ms delay before reading next character
    }
    if (blebuf[0] != 0) {
      //blebuf[i]=0;
      Serial.print("Buffer parsed with "); Serial.print(i); Serial.println(" characters");
      parseBLE(blebuf, BLEstrings);
      for(int n = 0; n < 4; n++)
       { 
        Serial.print(BLEstrings[n]); Serial.print("      -     ");
       }
       Serial.println();
       memset(blebuf,0,100);         // clear buffer
       i=0;
    }
  }   




  

}
